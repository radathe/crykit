#include "tea.hpp"

#include "crykit.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>

namespace cipher::tea {

    const AlgorithmInfo tea_info = {
        "tea",
        KEY_BYTES,
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

    void expand_key(ConstBuffer key, uint32_t words[KEY_WORDS]) {
        for (int i = 0; i < KEY_WORDS; ++i) {
            words[i] = load_word(key.data + i * WORD_BYTES);
        }
    }

    void encrypt_block(const uint32_t key[KEY_WORDS], const uint8_t* input, uint8_t* output) {
        uint32_t v0 = load_word(input);
        uint32_t v1 = load_word(input + WORD_BYTES);
        uint32_t sum = 0;
        for (int round = 0; round < ROUNDS; ++round) {
            sum += DELTA;
            v0 += ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
            v1 += ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
        }
        store_word(v0, output);
        store_word(v1, output + WORD_BYTES);
    }

    void decrypt_block(const uint32_t key[KEY_WORDS], const uint8_t* input, uint8_t* output) {
        uint32_t v0 = load_word(input);
        uint32_t v1 = load_word(input + WORD_BYTES);
        uint32_t sum = DELTA * ROUNDS;
        for (int round = 0; round < ROUNDS; ++round) {
            v1 -= ((v0 << 4) + key[2]) ^ (v0 + sum) ^ ((v0 >> 5) + key[3]);
            v0 -= ((v1 << 4) + key[0]) ^ (v1 + sum) ^ ((v1 >> 5) + key[1]);
            sum -= DELTA;
        }
        store_word(v0, output);
        store_word(v1, output + WORD_BYTES);
    }

    extern "C" const AlgorithmInfo* get_algorithm_info() {
        return &tea_info;
    }

    extern "C" size_t get_output_size(size_t input_size, int operation_type) {
        if (operation_type == CRYKIT_OP_ENCRYPT) {
            return (input_size / BLOCK_BYTES + 1) * BLOCK_BYTES;
        }
        return input_size;
    }

    extern "C" int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
        if (key.size != KEY_BYTES) {
            return CRYKIT_ERR_KEY;
        }
        if (output == nullptr || output->data == nullptr) {
            return CRYKIT_ERR_OUTPUT;
        }

        uint32_t words[KEY_WORDS];
        expand_key(key, words);

        const int status = utils::do_padding(input, BLOCK_BYTES, output);
        if (status != CRYKIT_OK) {
            return status;
        }

        for (size_t offset = 0; offset < output->size; offset += BLOCK_BYTES) {
            encrypt_block(words, output->data + offset, output->data + offset);
        }
        return CRYKIT_OK;
    }

    extern "C" int decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
        if (key.size != KEY_BYTES) {
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

        uint32_t words[KEY_WORDS];
        expand_key(key, words);

        for (size_t offset = 0; offset < input.size; offset += BLOCK_BYTES) {
            decrypt_block(words, input.data + offset, output->data + offset);
        }
        output->size = input.size;

        return utils::undo_padding(output, BLOCK_BYTES);
    }

}
