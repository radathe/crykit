
add_executable(crykit
    main.cpp
    loader.cpp
)

target_include_directories(crykit PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(crykit PRIVATE ${CMAKE_DL_LIBS})
