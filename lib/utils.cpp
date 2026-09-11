#include "utils.hpp"
#include "crykit.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace utils{
    void do_padding(ConstBuffer input, size_t block_size, MutBuffer& output){
        if (block_size == 0 || block_size > 255) {
            throw std::invalid_argument("PKCS#7 padding: block_size must be in range [1, 255]");
        }
        size_t data_len = block_size-(input.size%block_size)+input.size;
        uint8_t padded_array[data_len] {0};



    }
}
