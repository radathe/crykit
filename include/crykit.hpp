#pragma once

#include <cstddef>
#include <cstdint>


enum CrykitOperation : int {
    CRYKIT_OP_ENCRYPT = 0,
    CRYKIT_OP_DECRYPT = 1,
};

//коды возврата: успех - 0, любая ошибка - отрицательное число
enum CrykitStatus : int {
    CRYKIT_OK = 0,
    CRYKIT_ERR_KEY = -1,      //неверный размер или формат ключа
    CRYKIT_ERR_INPUT = -2,    //некорректные входные данные
    CRYKIT_ERR_PADDING = -3,  //некорректный или отсутствующий PKCS#7 padding
    CRYKIT_ERR_OUTPUT = -4,   // выходной буфер слишком мал
    CRYKIT_ERR_INTERNAL = -5, // прочие внутренние ошибки
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
