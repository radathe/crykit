#pragma once

#include <cstddef>
#include <cstdint>

enum CrykitOperation : int {
    CRYKIT_OP_ENCRYPT = 0,
    CRYKIT_OP_DECRYPT = 1,
};

enum CrykitStatus : int {
    CRYKIT_OK = 0,
    CRYKIT_ERR_KEY = -1,
    CRYKIT_ERR_INPUT = -2,
    CRYKIT_ERR_PADDING = -3,
    CRYKIT_ERR_OUTPUT = -4,
    CRYKIT_ERR_INTERNAL = -5,
};

struct ConstBuffer {
    const uint8_t* data;
    size_t size;
};

struct MutBuffer {
    uint8_t* data;
    size_t size;
};

struct AlgorithmInfo {
    const char* algorithm_name;
    size_t key_size;
};

extern "C" const AlgorithmInfo* get_algorithm_info();

extern "C" size_t get_output_size(size_t input_size, int operation_type);

extern "C" int encrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output);
extern "C" int decrypt(ConstBuffer key, ConstBuffer input, MutBuffer* output);
