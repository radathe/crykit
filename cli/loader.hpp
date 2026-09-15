#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "crykit.hpp"


typedef const AlgorithmInfo* (*GetInfoFn)();
typedef size_t (*GetOutputSizeFn)(size_t, int);
typedef int (*EncryptFn)(ConstBuffer, ConstBuffer, MutBuffer*);
typedef int (*DecryptFn)(ConstBuffer, ConstBuffer, MutBuffer*);


struct CipherModule {
    void* handle;
    std::string path;
    GetInfoFn get_info;
    GetOutputSizeFn get_output_size;
    EncryptFn encrypt;
    DecryptFn decrypt;
};


bool load_cipher_module(CipherModule& module,
                        const std::string& algorithm,
                        const std::vector<std::string>& search_dirs,
                        std::string& error);

void unload_cipher_module(CipherModule& module);
