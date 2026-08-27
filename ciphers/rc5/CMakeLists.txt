add_library(rc5 SHARED
    rc5.cpp
)

target_include_directories(rc5 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
