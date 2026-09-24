add_executable(test_rc5 test_rc5.cpp)
target_link_libraries(test_rc5 PRIVATE rc5)
target_include_directories(test_rc5 PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME rc5_encrypt_decrypt COMMAND test_rc5)

add_executable(test_tea test_tea.cpp)
target_link_libraries(test_tea PRIVATE tea)
target_include_directories(test_tea PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME tea_encrypt_decrypt COMMAND test_tea)

add_executable(test_hill test_hill.cpp)
target_link_libraries(test_hill PRIVATE hill)
target_include_directories(test_hill PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME hill_encrypt_decrypt COMMAND test_hill)
