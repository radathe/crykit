#pragma once

#include <cstddef>
#include <cstdint>

#include "crykit.hpp"

//параметры TEA: 64-битный блок, 128-битный ключ, 32 раунда.
namespace cipher::tea {

    constexpr int KEY_WORDS = 4;
    constexpr int WORD_BYTES = 4;
    constexpr int KEY_BYTES = KEY_WORDS * WORD_BYTES;
    constexpr int BLOCK_BYTES = WORD_BYTES * 2;
    constexpr int ROUNDS = 32;

    constexpr uint32_t DELTA = 0x9E3779B9u;

    //ключ — четыре 32-битных слова в порядке little-endian.
    void expand_key(ConstBuffer key, uint32_t words[KEY_WORDS]);

    void encrypt_block(const uint32_t key[KEY_WORDS], const uint8_t* input, uint8_t* output);
    void decrypt_block(const uint32_t key[KEY_WORDS], const uint8_t* input, uint8_t* output);

}
