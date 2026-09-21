#include "hill.hpp"

#include "crykit.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <stdexcept>

namespace cipher::hill {

    const AlgorithmInfo hill_info = {
        "hill",
        KEY_BYTES,
    };

    Matrix<GF28> keygen() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint8_t> dist(1, 255);
        while (true) {
            Matrix<GF28> key(N, N);
            for (size_t i = 0; i < N; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    key.at(i, j) = GF28(static_cast<uint8_t>(dist(rng)));
                }
            }
            try {
                key.inv();
                return key;
            } catch (const std::runtime_error&) {
            }
        }
    }

    Matrix<GF28> keyget(ConstBuffer key) {
        if (key.size != KEY_BYTES) {
            throw std::invalid_argument("Invalid key: wrong size");
        }
        Matrix<GF28> matrix(N, N);
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                matrix.at(i, j) = key.data[i * N + j];
            }
        }
        return matrix;
    }

    Matrix<GF28> block_to_matrix(const uint8_t* data, size_t offset) {
        Matrix<GF28> matrix(N, 1);
        for (size_t i = 0; i < N; ++i) {
            matrix.at(i, 0) = GF28(data[offset + i]);
        }
        return matrix;
    }

    void matrix_to_block(const Matrix<GF28>& matrix, uint8_t* result) {
        for (size_t i = 0; i < N; ++i) {
            result[i] = matrix.at(i, 0).value();
        }
    }

    Matrix<GF28> encrypt_block(const Matrix<GF28>& key, const Matrix<GF28>& block) {
        return key * block;
    }

    extern "C" int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output) {
        if (key.size != KEY_BYTES) {
            return CRYKIT_ERR_KEY;
        }
        if (output == nullptr || output->data == nullptr) {
            return CRYKIT_ERR_OUTPUT;
        }

        const Matrix<GF28> matrix_key = keyget(key);
        const int status = utils::do_padding(input, N, output);
        if (status != CRYKIT_OK) {
            return status;
        }

        for (size_t offset = 0; offset < output->size; offset += N) {
            const Matrix<GF28> block = block_to_matrix(output->data, offset);
            const Matrix<GF28> encrypted = encrypt_block(matrix_key, block);
            uint8_t result[N] = {0};
            matrix_to_block(encrypted, result);
            std::memcpy(output->data + offset, result, N);
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
        if (input.size == 0 || input.size % N != 0) {
            return CRYKIT_ERR_INPUT;
        }
        if (output->size < input.size) {
            return CRYKIT_ERR_OUTPUT;
        }

        Matrix<GF28> inverse_key;
        try {
            inverse_key = keyget(key).inv();
        } catch (const std::exception&) {
            return CRYKIT_ERR_KEY;
        }

        for (size_t offset = 0; offset < input.size; offset += N) {
            const Matrix<GF28> block = block_to_matrix(input.data, offset);
            const Matrix<GF28> decrypted = encrypt_block(inverse_key, block);
            uint8_t result[N] = {0};
            matrix_to_block(decrypted, result);
            std::memcpy(output->data + offset, result, N);
        }
        output->size = input.size;
        return utils::undo_padding(output, N);
    }

    extern "C" const AlgorithmInfo* get_algorithm_info() {
        return &hill_info;
    }

    extern "C" size_t get_output_size(size_t input_size, int operation_type) {
        if (operation_type == CRYKIT_OP_ENCRYPT) {
            return (input_size / N + 1) * N;
        }
        return input_size;
    }

}
