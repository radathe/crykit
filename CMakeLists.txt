cmake_minimum_required(VERSION 3.10)
project(crykit CXX)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(lib)
add_subdirectory(ciphers/hill)
add_subdirectory(ciphers/rc5)
add_subdirectory(ciphers/tea)

add_executable(crykit cli/main.cpp cli/loader.cpp)
target_include_directories(crykit PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(crykit PRIVATE utils)

if(UNIX)
    target_link_libraries(crykit PRIVATE ${CMAKE_DL_LIBS})
endif()

add_subdirectory(tests)
