add_executable(test_hill test_hill.cpp)
target_link_libraries(test_hill PRIVATE ciphers)
add_test(NAME Hill_encrypt_decrypt COMMAND test_hill)
target_include_directories(test_hill PUBLIC ${PROJECT_SOURCE_DIR}/include)
