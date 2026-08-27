add_executable(cryptum
    main.cpp
)

target_include_directories(cryptum PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(cryptum PRIVATE
    utils
)
