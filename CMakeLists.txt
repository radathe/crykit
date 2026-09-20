cmake_minimum_required(VERSION 3.10)
project(crykit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Повышенный уровень предупреждений, как рекомендует ТЗ.
add_compile_options(-Wall -Wextra)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Бинарник и разделяемые библиотеки складываются в один каталог: так основной
# модуль находит lib<algorithm>.so рядом с собой.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Вспомогательный код, используемый реализациями алгоритмов.
add_subdirectory(lib)

# Реализации алгоритмов — отдельные динамические библиотеки.
add_subdirectory(ciphers/hill)
add_subdirectory(ciphers/rc5)
add_subdirectory(ciphers/tea)

# Основной исполняемый модуль (CLI + загрузчик).
add_subdirectory(cli)
