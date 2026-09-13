#include "hill.hpp"

#include <cstdio>
#include <getopt.h>
#include <string>

struct Config {
    std::string cipher_name;  // --cipher / -c
    bool encrypt_mode = true; // --encrypt (default) / --decrypt
    std::string input_file;   // позиционный аргумент, или stdin
    std::string output_file;  // --output / -o, или stdout
    std::string key_file;     // --key / -k


    std::string key_out_file; // --key-out / -K
};

static void print_usage(const char* prog) {
    printf(
        "Использование: %s [опции] [<ввод>]\n"
        "\n"
        "Утилита для шифрования и дешифровки файлов с помощью выбранного шифра\n"
        "\n"
        "Общие опции:\n"
        "  -c, --cipher <name>   Используемый шифр (skel, otp, …)\n"
        "  -e, --encrypt         Режим шифрования, взаимоискл. с -d "
        "(стандартн.)\n"
        "  -d, --decrypt         Режим дешифрования, взаимоискл. с -e \n"
        "  -o, --output <file>   Записать вывод в файл (иначе: stdout)\n"
        "  -k, --key <file>      Считать ключ из файла\n"
        "  -K, --key-out <file>  Записать сгенерированный ключ в файл\n"
        "  -h, --help            Показать это сообщение\n"
        "\n"
        "Если <ввод> не указан, считывает ввод из stdin.\n",
        prog);
}

const static struct option opts[] = {
    {"cipher", required_argument, nullptr, 'c'},
    {"encrypt", no_argument, nullptr, 'e'},
    {"decrypt", no_argument, nullptr, 'd'},
    {"output", required_argument, nullptr, 'o'},
    {"key", required_argument, nullptr, 'k'},
    {"key-out", required_argument, nullptr, 'K'},
    {"help", no_argument, nullptr, 'h'},
    {nullptr, 0, nullptr, 0},
};
int main(int argc, char** argv){
    Config cfg;
    int opt;

    while((opt = getopt_long(argc, argv, "c:edo:k:K:h", opts, nullptr))!= -1){
        switch (opt) {
            case 'c':

        }
    }
}
