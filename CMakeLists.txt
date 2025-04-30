cmake_minimum_required(VERSION 3.16)
project(lab04 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 21)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включить все предупреждения
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Пути
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_SOURCE_DIR}/includes")
set(SQLITE_DIR "${CMAKE_SOURCE_DIR}/sqlite3")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")
set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin")

include_directories(${INC_DIR} ${SQLITE_DIR})

# Источники (ВСЕ .cpp)
file(GLOB SOURCES ${SRC_DIR}/*.cpp)

# Целевой исполняемый файл
add_executable(lab04 ${SOURCES})

# Подключаем локальную SQLite3
add_library(sqlite3 STATIC ${SQLITE_DIR}/sqlite3.c)
target_link_libraries(lab04 sqlite3)

# Если Windows — подключаем .lib
if (WIN32)
    target_link_libraries(lab04 ${SQLITE_DIR}/sqlite3.lib)
endif()

# Установить каталог сборки бинарника
set_target_properties(lab04 PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Установить директорию для установки (make install)
install(TARGETS lab04 RUNTIME DESTINATION bin)
