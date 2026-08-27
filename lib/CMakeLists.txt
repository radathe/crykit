add_library(utils STATIC
    utils.cpp
)
target_include_directories(utils PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
