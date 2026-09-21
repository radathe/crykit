#pragma once

#include <array>
#include <cstdint>

#include "crykit.hpp"

//параметры варианта RC5-32/12/16: 32-битные слова, 12 раундов, ключ 16 байт
namespace cipher::rc5 {

    constexpr int W = 32;
    constexpr int R = 12;
    constexpr int B = 16;

    constexpr int WORD_BYTES = W / 8;
    constexpr int BLOCK_BYTES = WORD_BYTES * 2;
    constexpr int S_WORDS = 2 * (R + 1);
    constexpr int L_WORDS = B / WORD_BYTES;

    //константы инициализации таблицы подключей: производные от e и золотого сечения.
    constexpr uint32_t P = 0xB7E15163u;
    constexpr uint32_t Q = 0x9E3779B9u;


    uint32_t rotate_left(uint32_t value, uint32_t shift);
    uint32_t rotate_right(uint32_t value, uint32_t shift);


    std::array<uint32_t, S_WORDS> expand_key(ConstBuffer key);

    void encrypt_block(const std::array<uint32_t, S_WORDS>& subkeys, const uint8_t* input, uint8_t* output);
    void decrypt_block(const std::array<uint32_t, S_WORDS>& subkeys, const uint8_t* input, uint8_t* output);

}
