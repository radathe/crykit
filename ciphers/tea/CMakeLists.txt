add_library(tea SHARED
    tea.cpp
)

target_include_directories(tea PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
