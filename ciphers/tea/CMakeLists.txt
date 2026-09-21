add_library(tea SHARED
    tea.cpp
)

target_include_directories(tea PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Библиотеке нужны общие функции padding из lib/.
target_link_libraries(tea PRIVATE utils)
