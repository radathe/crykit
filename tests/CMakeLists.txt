add_executable(test_hill test_hill.cpp)
target_link_libraries(test_hill PRIVATE hill)
target_include_directories(test_hill PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME hill_encrypt_decrypt COMMAND test_hill)
