#pragma once

#include <cstddef>
#include <cstdint>

#include "crykit.hpp"
#include "utils.hpp"

namespace cipher::hill {

    constexpr size_t N = 8;
    constexpr size_t KEY_BYTES = N * N;
    constexpr size_t BLOCK_BYTES = N;

    Matrix<GF28> keyget(ConstBuffer key);
    Matrix<GF28> block_to_matrix(const uint8_t* data, size_t offset);
    void matrix_to_block(const Matrix<GF28>& matrix, uint8_t* result);
    Matrix<GF28> encrypt_block(const Matrix<GF28>& key, const Matrix<GF28>& block);

}
