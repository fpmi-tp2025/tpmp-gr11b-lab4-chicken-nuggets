cmake_minimum_required(VERSION 3.30)
project(FleetDatabaseApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
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

include_directories(${INC_DIR} "${SQLITE_DIR}")

# Источники (ВСЕ .cpp)
file(GLOB SOURCES ${SRC_DIR}/*.cpp)

# Целевой исполняемый файл
add_executable(fleet_app "${SOURCES}")

# SQLite3 (внешняя или системная)
set(SQLITE3_INCLUDE_DIR "${SQLITE_DIR}")
set(SQLITE3_LIBRARY "${SQLITE_DIR}/sqlite3.lib")

include_directories(${SQLITE3_INCLUDE_DIR})
target_link_libraries(fleet_app ${SQLITE3_LIBRARY})


# Установить каталог сборки бинарника
set_target_properties(fleet_app PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Установить директорию для установки (make install)
install(TARGETS fleet_app RUNTIME DESTINATION bin)
