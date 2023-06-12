cmake_minimum_required(VERSION 3.0)
project(Biblioteca)

# Incluir las bibliotecas requeridas
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# Agregar los archivos fuente de tu programa
set(SOURCES Main.c)

# Generar el ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Enlazar las bibliotecas necesarias
target_link_libraries(${PROJECT_NAME} SQLite::SQLite3 Threads::Threads)
