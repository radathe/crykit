add_library(utils SHARED
    utils.cpp
)
target_include_directories(utils PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
