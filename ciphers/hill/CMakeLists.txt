add_library(hill SHARED
    hill.cpp
)

target_include_directories(hill PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(hill PRIVATE utils)
