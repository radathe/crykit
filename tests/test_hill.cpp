#include "hill.hpp"

#include "crykit.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

static int failures = 0;

static void check(bool condition, const std::string& name) {
    if (!condition) {
        std::printf("FAIL: %s\n", name.c_str());
        ++failures;
    }
}

static std::vector<uint8_t> identity_key() {
    std::vector<uint8_t> key(cipher::hill::KEY_BYTES, 0);
    for (size_t i = 0; i < cipher::hill::N; ++i) {
        key[i * cipher::hill::N + i] = 1;
    }
    return key;
}

static std::vector<uint8_t> triangular_key() {
    std::vector<uint8_t> key(cipher::hill::KEY_BYTES, 0);
    for (size_t i = 0; i < cipher::hill::N; ++i) {
        for (size_t j = i; j < cipher::hill::N; ++j) {
            key[i * cipher::hill::N + j] = static_cast<uint8_t>(i + j + 1);
        }
    }
    return key;
}

static bool roundtrip(const std::vector<uint8_t>& key, size_t length) {
    std::vector<uint8_t> plain(length);
    for (size_t i = 0; i < plain.size(); ++i) {
        plain[i] = static_cast<uint8_t>(i * 37 + 11);
    }

    const size_t cipher_size = (length / cipher::hill::BLOCK_BYTES + 1) * cipher::hill::BLOCK_BYTES;
    std::vector<uint8_t> cipher(cipher_size);
    ConstBuffer key_buffer{key.data(), key.size()};
    MutBuffer cipher_buffer{cipher.data(), cipher.size()};
    if (encrypt(key_buffer, ConstBuffer{plain.data(), plain.size()}, &cipher_buffer) != CRYKIT_OK) {
        return false;
    }

    std::vector<uint8_t> restored(cipher_buffer.size);
    MutBuffer restored_buffer{restored.data(), restored.size()};
    if (decrypt(key_buffer, ConstBuffer{cipher.data(), cipher_buffer.size}, &restored_buffer) != CRYKIT_OK) {
        return false;
    }
    if (restored_buffer.size != plain.size()) {
        return false;
    }
    for (size_t i = 0; i < plain.size(); ++i) {
        if (restored[i] != plain[i]) {
            return false;
        }
    }
    return true;
}

static void test_identity_key() {
    const std::vector<uint8_t> key = identity_key();
    std::vector<uint8_t> plain(cipher::hill::BLOCK_BYTES);
    for (size_t i = 0; i < plain.size(); ++i) {
        plain[i] = static_cast<uint8_t>(i + 1);
    }

    std::vector<uint8_t> cipher(2 * cipher::hill::BLOCK_BYTES);
    MutBuffer cipher_buffer{cipher.data(), cipher.size()};
    const int status = encrypt(ConstBuffer{key.data(), key.size()},
                               ConstBuffer{plain.data(), plain.size()}, &cipher_buffer);

    bool ok = status == CRYKIT_OK && cipher_buffer.size == 2 * cipher::hill::BLOCK_BYTES;
    for (size_t i = 0; ok && i < plain.size(); ++i) {
        if (cipher[i] != plain[i]) {
            ok = false;
        }
    }
    for (size_t i = plain.size(); ok && i < cipher_buffer.size; ++i) {
        if (cipher[i] != static_cast<uint8_t>(cipher::hill::BLOCK_BYTES)) {
            ok = false;
        }
    }
    check(ok, "identity key keeps data and appends PKCS#7 padding");
}

int main() {
    test_identity_key();

    const std::vector<uint8_t> keys[] = {identity_key(), triangular_key()};
    const size_t lengths[] = {0, 1, 7, 8, 9, 16, 17, 255};
    for (size_t k = 0; k < sizeof(keys) / sizeof(keys[0]); ++k) {
        for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
            check(roundtrip(keys[k], lengths[i]), "roundtrip len=" + std::to_string(lengths[i]));
        }
    }

    std::vector<uint8_t> short_key(cipher::hill::KEY_BYTES - 1, 1);
    std::vector<uint8_t> block(cipher::hill::BLOCK_BYTES, 0);
    uint8_t out[2 * cipher::hill::BLOCK_BYTES];
    MutBuffer buffer{out, sizeof(out)};
    check(encrypt(ConstBuffer{short_key.data(), short_key.size()},
                  ConstBuffer{block.data(), block.size()}, &buffer) == CRYKIT_ERR_KEY,
          "reject short key");

    const std::vector<uint8_t> key = identity_key();
    check(decrypt(ConstBuffer{key.data(), key.size()},
                  ConstBuffer{block.data(), cipher::hill::BLOCK_BYTES - 1}, &buffer) == CRYKIT_ERR_INPUT,
          "reject non-block ciphertext");

    const std::vector<uint8_t> singular(cipher::hill::KEY_BYTES, 0);
    MutBuffer singular_buffer{out, sizeof(out)};
    check(decrypt(ConstBuffer{singular.data(), singular.size()},
                  ConstBuffer{block.data(), block.size()}, &singular_buffer) == CRYKIT_ERR_KEY,
          "reject singular key");

    if (failures != 0) {
        std::printf("hill: %d test(s) failed\n", failures);
        return 1;
    }
    std::printf("hill: all tests passed\n");
    return 0;
}
