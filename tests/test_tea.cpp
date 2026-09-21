#include "tea.hpp"

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

static bool roundtrip(size_t length, uint8_t seed) {
    std::vector<uint8_t> key(cipher::tea::KEY_BYTES);
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(seed + i);
    }
    std::vector<uint8_t> plain(length);
    for (size_t i = 0; i < plain.size(); ++i) {
        plain[i] = static_cast<uint8_t>(seed * 7 + i * 13);
    }

    const size_t cipher_size = (length / cipher::tea::BLOCK_BYTES + 1) * cipher::tea::BLOCK_BYTES;
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

int main() {
    const uint8_t zero_key[cipher::tea::KEY_BYTES] = {0};
    const uint8_t zero_block[cipher::tea::BLOCK_BYTES] = {0};
    //вектор: 41EA3A0A 94BAA940
    const uint8_t expected[cipher::tea::BLOCK_BYTES] = {
        0x0A, 0x3A, 0xEA, 0x41, 0x40, 0xA9, 0xBA, 0x94,
    };

    uint32_t words[cipher::tea::KEY_WORDS];
    cipher::tea::expand_key(ConstBuffer{zero_key, sizeof(zero_key)}, words);
    uint8_t cipher[cipher::tea::BLOCK_BYTES];
    cipher::tea::encrypt_block(words, zero_block, cipher);
    bool vector_ok = true;
    for (int i = 0; i < cipher::tea::BLOCK_BYTES; ++i) {
        if (cipher[i] != expected[i]) {
            vector_ok = false;
        }
    }
    check(vector_ok, "official test vector");

    const size_t lengths[] = {0, 1, 7, 8, 9, 16, 17, 255};
    for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
        check(roundtrip(lengths[i], static_cast<uint8_t>(i + 1)),
              "roundtrip len=" + std::to_string(lengths[i]));
    }

    uint8_t out[2 * cipher::tea::BLOCK_BYTES];
    MutBuffer buffer{out, sizeof(out)};
    check(encrypt(ConstBuffer{zero_key, cipher::tea::KEY_BYTES - 1},
                  ConstBuffer{zero_block, cipher::tea::BLOCK_BYTES}, &buffer) == CRYKIT_ERR_KEY,
          "reject short key");
    check(decrypt(ConstBuffer{zero_key, cipher::tea::KEY_BYTES},
                  ConstBuffer{zero_block, cipher::tea::BLOCK_BYTES - 1}, &buffer) == CRYKIT_ERR_INPUT,
          "reject non-block ciphertext");

    if (failures != 0) {
        std::printf("tea: %d test(s) failed\n", failures);
        return 1;
    }
    std::printf("tea: all tests passed\n");
    return 0;
}
