addlibrary (ciphers SHARED
    hill.cpp
)

target_link_libraries(ciphers PRIVATE
    utils
)

target_include_directories(ciphers PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
