#include "hill.hpp"
#include "utils.hpp"
#include "crykit.hpp"
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <random>

namespace  cipher::hill {

    const size_t N = 8;
    const AlgorithmInfo hill_info = {
        "hill",
        N*N,
    };
    enum mode {
        ENCRYPT = 0,
        DECRYPT = 1
    };
    Matrix<GF28> keygen(){
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint8_t> dist(1, 255);
        while (true) {
            Matrix<GF28> key(N,N);
            for (size_t i = 0; i <N; ++i)
                for (size_t j = 0; j <N; ++j){
                    key.at(i, j) = GF28(uint8_t(dist(rng)));
                }
            try {
                key.inv();
                return key;
            } catch (const std::runtime_error&) {
            }

        }
    }

    Matrix<GF28> keyget(ConstBuffer key){
        if (N*N != key.size) {
            throw std::invalid_argument("Invalid key: matrix is not square");
        }
        Matrix<GF28> m(N,N);
        for (size_t i = 0; i<N; ++i){
            for (size_t j = 0; j<N; ++j){
                m.at(i, j) = key.data[i*N+j];
            }
        }
        return m;
    }
    Matrix<GF28> block_to_matrix(uint8_t* data,
        size_t offset){
            Matrix<GF28> m(N, 1);
            for (size_t i=0; i<N; i++){
                m.at(i, 0) = GF28(static_cast<uint8_t>(data[offset+i]));
            }
            return m;
    }
    uint8_t matrix_to_block(Matrix<GF28>){}
    Matrix<GF28> encrypt_block(const Matrix<GF28> &key, const Matrix<GF28> &block){
        return key*block;
    }
    extern "C" int encrypt(ConstBuffer key,
        ConstBuffer input,
        MutBuffer* output){
            Matrix<GF28> matrix_key = keyget(key);

    }

    extern "C" const AlgorithmInfo* get_algorithm_info(){
        return &hill_info;
    }

    extern "C" size_t get_output_size(size_t input_size, int operation_type){
        if (operation_type == mode::ENCRYPT) {
            if (input_size/N*N == 0) return input_size;
            else return (input_size/(N*N)+1)*N*N;
        }
        else return input_size;
    }


}
