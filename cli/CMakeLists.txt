# Основной исполняемый модуль: разбор аргументов, ввод-вывод и динамическая
# загрузка библиотек алгоритмов. Конкретных шифров здесь нет — они подключаются
# во время выполнения через C-интерфейс из include/crykit.hpp.
add_executable(crykit
    main.cpp
    loader.cpp
)

target_include_directories(crykit PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# dlopen/dlsym/dlclose. ${CMAKE_DL_LIBS} подставляет libdl там, где она нужна,
# и остаётся пустым на системах, где эти функции уже в libc.
target_link_libraries(crykit PRIVATE ${CMAKE_DL_LIBS})
