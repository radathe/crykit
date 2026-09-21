#include "rc5.hpp"

#include "crykit.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <array>


namespace cipher::rc5 {

    const AlgorithmInfo rc5_info = {
        "rc5",
        B,
    };

    static uint32_t load_word(const uint8_t* data) {
        return static_cast<uint32_t>(data[0]) |
               (static_cast<uint32_t>(data[1]) << 8) |
               (static_cast<uint32_t>(data[2]) << 16) |
               (static_cast<uint32_t>(data[3]) << 24);
    }

    static void store_word(uint32_t value, uint8_t* data) {
        data[0] = static_cast<uint8_t>(value);
        data[1] = static_cast<uint8_t>(value >> 8);
        data[2] = static_cast<uint8_t>(value >> 16);
        data[3] = static_cast<uint8_t>(value >> 24);
    }

    uint32_t rotate_left(uint32_t value, uint32_t shift) {
        shift %= W;
        if (shift == 0) {
            return value;
        }
        return (value << shift) | (value >> (W - shift));
    }

    uint32_t rotate_right(uint32_t value, uint32_t shift) {
        shift %= W;
        if (shift == 0) {
            return value;
        }
        return (value >> shift) | (value << (W - shift));
    }

    std::array<uint32_t, S_WORDS> expand_key(ConstBuffer key) {
        uint32_t words[L_WORDS] = {0};
        for (size_t i = 0; i < key.size; ++i) {
            words[i / WORD_BYTES] |= static_cast<uint32_t>(key.data[i])
                                     << (8 * (i % WORD_BYTES));
        }

        std::array<uint32_t, S_WORDS> subkeys{};
        subkeys[0] = P;
        for (size_t i = 1; i < S_WORDS; ++i) {
            subkeys[i] = subkeys[i - 1] + Q;
        }

        //перемешивание таблицы подключей с ключом число проходов: 3 * max(t, c), где t = S_WORDS, c = L_WORDS.
        constexpr size_t STEPS = 3 * (S_WORDS > L_WORDS ? S_WORDS : L_WORDS);
        uint32_t a = 0;
        uint32_t b = 0;
        size_t i = 0;
        size_t j = 0;
        for (size_t step = 0; step < STEPS; ++step) {
            a = subkeys[i] = rotate_left(subkeys[i] + a + b, 3);
            b = words[j] = rotate_left(words[j] + a + b, a + b);
            i = (i + 1) % S_WORDS;
            j = (j + 1) % L_WORDS;
        }
        return subkeys;
    }

    void encrypt_block(const std::array<uint32_t, S_WORDS>& subkeys, const uint8_t* input, uint8_t* output) {
        uint32_t a = load_word(input);
        uint32_t b = load_word(input + WORD_BYTES);

        a += subkeys[0];
        b += subkeys[1];
        for (int round = 1; round <= R; ++round) {
            a = rotate_left(a ^ b, b) + subkeys[2 * round];
            b = rotate_left(b ^ a, a) + subkeys[2 * round + 1];
        }

        store_word(a, output);
        store_word(b, output + WORD_BYTES);
    }

    void decrypt_block(const std::array<uint32_t, S_WORDS>& subkeys, const uint8_t* input, uint8_t* output) {
        uint32_t a = load_word(input);
        uint32_t b = load_word(input + WORD_BYTES);

        for (int round = R; round >= 1; --round) {
            b = rotate_right(b - subkeys[2 * round + 1], a) ^ a;
            a = rotate_right(a - subkeys[2 * round], b) ^ b;
        }
        b -= subkeys[1];
        a -= subkeys[0];

        store_word(a, output);
        store_word(b, output + WORD_BYTES);
    }

    extern "C" const AlgorithmInfo* get_algorithm_info() {
        return &rc5_info;
    }

    extern "C" size_t get_output_size(size_t input_size, int operation_type) {
        if (operation_type == CRYKIT_OP_ENCRYPT) {
            return (input_size / BLOCK_BYTES + 1) * BLOCK_BYTES;
        }
        return input_size;
    }

    extern "C" int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
        if (key.size != B) {
            return CRYKIT_ERR_KEY;
        }
        if (output == nullptr || output->data == nullptr) {
            return CRYKIT_ERR_OUTPUT;
        }

        const std::array<uint32_t, S_WORDS> subkeys = expand_key(key);
        const int status = utils::do_padding(input, BLOCK_BYTES, output);
        if (status != CRYKIT_OK) {
            return status;
        }

        for (size_t offset = 0; offset < output->size; offset += BLOCK_BYTES) {
            encrypt_block(subkeys, output->data + offset, output->data + offset);
        }
        return CRYKIT_OK;
    }

    extern "C" int decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
        if (key.size != B) {
            return CRYKIT_ERR_KEY;
        }
        if (output == nullptr || output->data == nullptr) {
            return CRYKIT_ERR_OUTPUT;
        }
        if (input.size == 0 || input.size % BLOCK_BYTES != 0) {
            return CRYKIT_ERR_INPUT;
        }
        if (output->size < input.size) {
            return CRYKIT_ERR_OUTPUT;
        }

        const std::array<uint32_t, S_WORDS> subkeys = expand_key(key);
        for (size_t offset = 0; offset < input.size; offset += BLOCK_BYTES) {
            decrypt_block(subkeys, input.data + offset, output->data + offset);
        }
        output->size = input.size;

        return utils::undo_padding(output, BLOCK_BYTES);
    }

}
