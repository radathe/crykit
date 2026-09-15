#include "loader.hpp"

#include <dlfcn.h>

#include <string>
#include <vector>

static std::string with_slash(const std::string& directory) {
    if (!directory.empty() && directory.back() != '/') {
        return directory + '/';
    }
    return directory;
}

static std::string library_file_name(const std::string& algorithm) {
    return "lib" + algorithm + ".so";
}

static bool looks_like_path(const std::string& algorithm) {
    if (algorithm.find('/') != std::string::npos) {
        return true;
    }
    const std::string suffix = ".so";
    return algorithm.size() > suffix.size() &&
           algorithm.compare(algorithm.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool load_cipher_module(CipherModule& module,
                        const std::string& algorithm,
                        const std::vector<std::string>& search_dirs,
                        std::string& error) {
    module = CipherModule();

    if (algorithm.empty()) {
        error = "не задано имя алгоритма";
        return false;
    }

    //список путей
    std::vector<std::string> candidates;
    if (looks_like_path(algorithm)) {
        candidates.push_back(algorithm);
    } else {
        const std::string file = library_file_name(algorithm);
        for (size_t i = 0; i < search_dirs.size(); ++i) {
            if (!search_dirs[i].empty()) {
                candidates.push_back(with_slash(search_dirs[i]) + file);
            }
        }
        candidates.push_back(file);
    }

    std::string last_error = "не найдена библиотека для алгоритма: " + algorithm;

    for (size_t i = 0; i < candidates.size(); ++i) {
        dlerror();
        void* handle = dlopen(candidates[i].c_str(), RTLD_NOW | RTLD_LOCAL);
        if (handle == nullptr) {
            const char* message = dlerror();
            last_error = "не удалось загрузить " + candidates[i] + ": " +
                         (message != nullptr ? message : "неизвестная ошибка");
            continue;
        }

        const char* names[] = {"get_algorithm_info", "get_output_size", "encrypt", "decrypt"};
        void* addresses[4] = {nullptr, nullptr, nullptr, nullptr};
        bool complete = true;
        for (int k = 0; k < 4; ++k) {
            addresses[k] = dlsym(handle, names[k]);
            if (addresses[k] == nullptr) {
                error = "в библиотеке " + candidates[i] + " отсутствует функция " + names[k];
                complete = false;
                break;
            }
        }
        if (!complete) {
            dlclose(handle);
            return false;
        }

        module.handle = handle;
        module.path = candidates[i];
        module.get_info = reinterpret_cast<GetInfoFn>(addresses[0]);
        module.get_output_size = reinterpret_cast<GetOutputSizeFn>(addresses[1]);
        module.encrypt = reinterpret_cast<EncryptFn>(addresses[2]);
        module.decrypt = reinterpret_cast<DecryptFn>(addresses[3]);
        return true;
    }

    error = last_error;
    return false;
}

void unload_cipher_module(CipherModule& module) {
    if (module.handle != nullptr) {
        dlclose(module.handle);
    }
    module = CipherModule();
}
