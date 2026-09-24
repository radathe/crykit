add_library(utils STATIC
    utils.cpp
)
target_include_directories(utils PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(utils PROPERTIES POSITION_INDEPENDENT_CODE ON)
