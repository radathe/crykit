#include "loader.hpp"

#include <getopt.h>
#include <unistd.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>


static const char* const PROGRAM_VERSION = "0.1.0";

//режим работы
enum Mode {
    MODE_NONE = 0,
    MODE_ENCRYPT,
    MODE_DECRYPT,
    MODE_GEN_KEY,
};

//аргументы cli
struct Options {
    int mode;
    std::string algorithm;
    std::string input;
    std::string output;
    std::string key;
    std::string library_dir;
    bool show_help;
    bool show_version;
    bool list_algorithms;
};

static void print_usage(const char* program) {
    std::cout
        << "Использование: " << program << " [опции]\n"
        << "\n"
        << "Шифрование и расшифрование данных из файлов или stdin с помощью динамически подключаемых\n"
        << "библиотек-алгоритмов.\n"
        << "\n"
        << "Режим работы (только один):\n"
        << "  -e, --encrypt            Зашифровать входные данные\n"
        << "  -d, --decrypt            Расшифровать входные данные\n"
        << "  -g, --gen-key            Сгенерировать ключ для алгоритма\n"
        << "\n"
        << "Алгоритм и данные:\n"
        << "  -a, --algorithm <name>   Алгоритм, например: hill, rc5, tea\n"
        << "  -k, --key <file>         Файл с ключом (обязателен для -e и -d)\n"
        << "  -i, --input <file>       Входной файл (по умолчанию stdin)\n"
        << "  -o, --output <file>      Выходной файл (по умолчанию stdout)\n"
        << "  -L, --library-dir <dir>  Дополнительный каталог поиска lib<algorithm>.so\n"
        << "\n"
        << "Прочее:\n"
        << "  -l, --list               Показать известные алгоритмы\n"
        << "  -V, --version            Показать версию\n"
        << "  -h, --help               Показать эту справку\n"
        << "\n"
        << "Вместо имени файла можно указать '-' для stdin/stdout.\n"
        << "\n"
        << "Примеры:\n"
        << "  " << program << " -a hill -g -o key.bin\n"
        << "  " << program << " -a hill -e -k key.bin -i in.bin -o out.bin\n";
}

static void print_algorithms() {
    static const char* const names[] = {"hill", "rc5", "tea"};
    const size_t count = sizeof(names) / sizeof(names[0]);

    std::cout << "Известные алгоритмы:\n";
    for (size_t i = 0; i < count; ++i) {
        std::cout << "  " << names[i] << "  (lib" << names[i] << ".so)\n";
    }
}

static bool parse_args(int argc, char** argv, Options& options, std::string& error) {
    options.mode = MODE_NONE;
    options.algorithm.clear();
    options.input.clear();
    options.output.clear();
    options.key.clear();
    options.library_dir.clear();
    options.show_help = false;
    options.show_version = false;
    options.list_algorithms = false;

    static const struct option long_options[] = {
        {"algorithm",   required_argument, nullptr, 'a'},
        {"encrypt",     no_argument,       nullptr, 'e'},
        {"decrypt",     no_argument,       nullptr, 'd'},
        {"gen-key",     no_argument,       nullptr, 'g'},
        {"key",         required_argument, nullptr, 'k'},
        {"input",       required_argument, nullptr, 'i'},
        {"output",      required_argument, nullptr, 'o'},
        {"library-dir", required_argument, nullptr, 'L'},
        {"list",        no_argument,       nullptr, 'l'},
        {"version",     no_argument,       nullptr, 'V'},
        {"help",        no_argument,       nullptr, 'h'},
        {nullptr, 0, nullptr, 0},
    };

    opterr = 0;
    int index = 0;
    int value = 0;
    while ((value = getopt_long(argc, argv, "a:edgk:i:o:L:lVh", long_options, &index)) != -1) {
        switch (value) {
            case 'a': options.algorithm = optarg; break;
            case 'e':
                if (options.mode != MODE_NONE) { error = "режим работы задан дважды"; return false; }
                options.mode = MODE_ENCRYPT;
                break;
            case 'd':
                if (options.mode != MODE_NONE) { error = "режим работы задан дважды"; return false; }
                options.mode = MODE_DECRYPT;
                break;
            case 'g':
                if (options.mode != MODE_NONE) { error = "режим работы задан дважды"; return false; }
                options.mode = MODE_GEN_KEY;
                break;
            case 'k': options.key = optarg; break;
            case 'i': options.input = optarg; break;
            case 'o': options.output = optarg; break;
            case 'L': options.library_dir = optarg; break;
            case 'l': options.list_algorithms = true; break;
            case 'V': options.show_version = true; break;
            case 'h': options.show_help = true; break;
            default:
                if (optopt != 0) {
                    error = std::string("неизвестная или неполная опция '-") +
                            static_cast<char>(optopt) + "'";
                } else if (optind > 0 && optind <= argc) {
                    error = std::string("неизвестный аргумент: ") + argv[optind - 1];
                } else {
                    error = "не удалось разобрать аргументы командной строки";
                }
                return false;
        }
    }

    if (optind < argc) {
        error = std::string("лишний аргумент: ") + argv[optind];
        return false;
    }

    if (options.show_help || options.show_version || options.list_algorithms) {
        return true;
    }

    if (options.mode == MODE_NONE) {
        error = "не выбран режим работы (укажите -e, -d или -g)";
        return false;
    }
    if (options.algorithm.empty()) {
        error = "не указан алгоритм (-a / --algorithm)";
        return false;
    }
    if ((options.mode == MODE_ENCRYPT || options.mode == MODE_DECRYPT) && options.key.empty()) {
        error = "не указан ключ (-k / --key) для режима шифрования/расшифрования";
        return false;
    }
    return true;
}

static bool read_all(const std::string& path, std::vector<uint8_t>& data, std::string& error) {
    data.clear();

    if (path.empty() || path == "-") {
        char byte = 0;
        while (std::cin.get(byte)) {
            data.push_back(static_cast<uint8_t>(byte));
        }
        if (std::cin.bad()) {
            error = "ошибка чтения из stdin";
            return false;
        }
        return true;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        error = "не удалось открыть входной файл: " + path;
        return false;
    }

    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    if (length < 0) {
        error = "не удалось определить размер файла: " + path;
        return false;
    }
    file.seekg(0, std::ios::beg);

    data.resize(static_cast<size_t>(length));
    if (length > 0) {
        file.read(reinterpret_cast<char*>(data.data()), length);
        if (!file) {
            error = "ошибка чтения файла: " + path;
            return false;
        }
    }
    return true;
}

static bool write_all(const std::string& path, const std::vector<uint8_t>& data, std::string& error) {
    if (path.empty() || path == "-") {
        if (!data.empty()) {
            std::cout.write(reinterpret_cast<const char*>(data.data()),
                            static_cast<std::streamsize>(data.size()));
        }
        std::cout.flush();
        if (!std::cout) {
            error = "ошибка записи в stdout";
            return false;
        }
        return true;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "не удалось открыть выходной файл: " + path;
        return false;
    }
    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()),
                   static_cast<std::streamsize>(data.size()));
    }
    file.flush();
    if (!file) {
        error = "ошибка записи файла: " + path;
        return false;
    }
    return true;
}


static std::string executable_directory() {
    char buffer[4096];
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        return std::string();
    }
    buffer[length] = '\0';

    const std::string path(buffer);
    const size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return std::string();
    }
    return path.substr(0, slash);
}

static const char* status_text(int status) {
    switch (status) {
        case CRYKIT_ERR_KEY: return "неправильный ключ";
        case CRYKIT_ERR_INPUT: return "некорректные входные данные";
        case CRYKIT_ERR_PADDING: return "некорректный padding";
        case CRYKIT_ERR_OUTPUT: return "недостаточный размер выходного буфера";
        case CRYKIT_ERR_INTERNAL: return "внутренняя ошибка алгоритма";
        default: return "неизвестная ошибка";
    }
}

static bool run_generate_key(const CipherModule& module, const Options& options, std::string& error) {
    const AlgorithmInfo* info = module.get_info();
    if (info == nullptr) {
        error = "библиотека вернула пустую информацию об алгоритме";
        return false;
    }
    if (info->key_size == 0) {
        error = "алгоритм не сообщает размер ключа, генерация невозможна";
        return false;
    }

    std::vector<uint8_t> key(info->key_size);
    std::random_device random;
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(random());
    }
    return write_all(options.output, key, error);
}

static bool run_cipher(const CipherModule& module,
                       const Options& options,
                       bool encrypt,
                       std::string& error) {
    const AlgorithmInfo* info = module.get_info();
    if (info == nullptr) {
        error = "библиотека вернула пустую информацию об алгоритме";
        return false;
    }

    std::vector<uint8_t> key;
    if (!read_all(options.key, key, error)) {
        return false;
    }
    // key_size = 0 - размер не фиксирован
    if (info->key_size != 0 && key.size() != info->key_size) {
        error = "неправильная длина ключа: ожидается " + std::to_string(info->key_size) +
                " байт, получено " + std::to_string(key.size());
        return false;
    }

    std::vector<uint8_t> input;
    if (!read_all(options.input, input, error)) {
        return false;
    }

    const int operation = encrypt ? CRYKIT_OP_ENCRYPT : CRYKIT_OP_DECRYPT;
    const size_t required = module.get_output_size(input.size(), operation);
    //минимум 1 байт для корректного указателя
    std::vector<uint8_t> output(required > 0 ? required : 1);

    ConstBuffer key_buffer{key.data(), key.size()};
    ConstBuffer input_buffer{input.data(), input.size()};
    MutBuffer output_buffer{output.data(), required};

    const int status = encrypt
        ? module.encrypt(key_buffer, input_buffer, &output_buffer)
        : module.decrypt(key_buffer, input_buffer, &output_buffer);

    if (status != CRYKIT_OK) {
        error = std::string(encrypt ? "шифрование" : "расшифрование") + ": " + status_text(status) +
                " (код " + std::to_string(status) + ")";
        return false;
    }

    //после снятия padding результат может быть короче выделенного буфера.
    const size_t produced = output_buffer.size < output.size() ? output_buffer.size : output.size();
    output.resize(produced);
    return write_all(options.output, output, error);
}

int main(int argc, char** argv) {
    Options options;
    std::string error;

    if (!parse_args(argc, argv, options, error)) {
        std::cerr << "crykit: " << error << "\n";
        std::cerr << "Подсказка: crykit --help\n";
        return 1;
    }

    if (options.show_help) {
        print_usage(argv[0]);
        return 0;
    }
    if (options.show_version) {
        std::cout << "crykit " << PROGRAM_VERSION << "\n";
        return 0;
    }
    if (options.list_algorithms) {
        print_algorithms();
        return 0;
    }

    //поиск библиотеки: каталог из -L, затем каталог исполняемого файла, затем текущий каталог
    std::vector<std::string> search_dirs;
    if (!options.library_dir.empty()) {
        search_dirs.push_back(options.library_dir);
    }
    const std::string self_dir = executable_directory();
    if (!self_dir.empty()) {
        search_dirs.push_back(self_dir);
    }
    search_dirs.push_back(".");

    CipherModule module;
    if (!load_cipher_module(module, options.algorithm, search_dirs, error)) {
        std::cerr << "crykit: " << error << "\n";
        return 1;
    }

    bool ok = false;
    if (options.mode == MODE_GEN_KEY) {
        ok = run_generate_key(module, options, error);
    } else {
        ok = run_cipher(module, options, options.mode == MODE_ENCRYPT, error);
    }

    unload_cipher_module(module);

    if (!ok) {
        std::cerr << "crykit: " << error << "\n";
        return 1;
    }
    return 0;
}
