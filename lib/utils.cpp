#include "utils.hpp"

#include <cstddef>
#include <cstdint>

#include "crykit.hpp"


namespace utils {


int do_padding(ConstBuffer input, size_t block_size, MutBuffer* output) {
    if (block_size == 0 || block_size > 255) {
        return CRYKIT_ERR_PADDING;
    }
    if (output == nullptr || output->data == nullptr) {
        return CRYKIT_ERR_OUTPUT;
    }

    const size_t pad_length = block_size - (input.size % block_size);
    const size_t padded_size = input.size + pad_length;

    if (output->size < padded_size) {
        return CRYKIT_ERR_OUTPUT;
    }

    for (size_t i = 0; i < input.size; ++i) {
        output->data[i] = input.data[i];
    }
    for (size_t i = input.size; i < padded_size; ++i) {
        output->data[i] = static_cast<uint8_t>(pad_length);
    }

    output->size = padded_size;
    return CRYKIT_OK;
}

int undo_padding(MutBuffer* output, size_t block_size) {
    if (block_size == 0 || block_size > 255) {
        return CRYKIT_ERR_PADDING;
    }
    if (output == nullptr || output->data == nullptr) {
        return CRYKIT_ERR_OUTPUT;
    }

    if (output->size == 0 || output->size % block_size != 0) {
        return CRYKIT_ERR_PADDING;
    }

    const size_t pad_length = output->data[output->size - 1];
    if (pad_length == 0 || pad_length > block_size || pad_length > output->size) {
        return CRYKIT_ERR_PADDING;
    }

    for (size_t i = output->size - pad_length; i < output->size; ++i) {
        if (output->data[i] != static_cast<uint8_t>(pad_length)) {
            return CRYKIT_ERR_PADDING;
        }
    }

    output->size -= pad_length;
    return CRYKIT_OK;
}

} //namespace utils
