#include "rc5.hpp"
#include "utils.hpp"
#include "crykit.hpp"

#include <cstddef>
#include <cstdint>

namespace cipher::rc5 {
    constexpr int W = 32;
    constexpr int R = 12;
    constexpr int B = 16;

    constexpr int WORD_BYTES = W / 8;
    constexpr int BLOCK_BYTES = WORD_BYTES * 2;
    constexpr int S_WORDS = 2*(R + 1);

    const AlgorithmInfo rc5_info = {
        "rc5",
        B,
    };

    struct RC5Key {
        uint32_t words[S_WORDS];
    };

    void key_expand(){

    }

    extern "C" int encrypt(ConstBuffer key,
        ConstBuffer input, MutBuffer *output) {

        }
    extern "C" int decrypt(ConstBuffer key,
        ConstBuffer input, MutBuffer *output) {

        }
    extern "C" const AlgorithmInfo* get_algorithm_info(){
        return rc5_info;
    }
    extern "C" size_t get_output_size(size_t input_size, int operation_type){

    }
}
