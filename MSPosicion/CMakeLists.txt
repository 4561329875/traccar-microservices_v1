cmake_minimum_required(VERSION 3.10)  # Ajustado para evitar la advertencia de CMake
project(microservice-positions CXX)

# Configurar el estándar de C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configurar vcpkg si está disponible
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Buscar Drogon
find_package(Drogon CONFIG REQUIRED)

# Buscar PostgreSQL
find_package(PostgreSQL REQUIRED)

# Buscar libpqxx de forma manual si no está en el sistema
find_path(PQXX_INCLUDE_DIR pqxx/pqxx)
find_library(PQXX_LIBRARY pqxx)

if(NOT PQXX_INCLUDE_DIR OR NOT PQXX_LIBRARY)
    message(FATAL_ERROR "libpqxx no encontrada. Instálala con: sudo apt install libpqxx-dev")
endif()

# Archivos fuente
set(SOURCE_FILES
    main.cc
    Position.cpp
    PositionController.cpp
    DatabaseConnection.cpp
)

# Crear el ejecutable principal
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Incluir directorios de PostgreSQL y libpqxx
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # Directorio actual del proyecto
    ${CMAKE_CURRENT_SOURCE_DIR}/models  # Si tienes subdirectorios para modelos
    ${PostgreSQL_INCLUDE_DIRS}  # Incluir PostgreSQL
    ${PQXX_INCLUDE_DIR}  # Incluir libpqxx
)

# Enlazar bibliotecas necesarias
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Drogon::Drogon  # Drogon
    ${PQXX_LIBRARY}  # Enlace con libpqxx
    pq  # Enlace con la biblioteca pq (PostgreSQL)
    ${PostgreSQL_LIBRARIES}  # Enlace con PostgreSQL
)

# Agregar vistas de Drogon (si es necesario)
drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR})

# Verificación del estándar de C++
if (CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "C++20 es requerido")
else ()
    message(STATUS "Usando C++20")
endif()

# Subdirectorio de pruebas (si es necesario)
#add_subdirectory(test)
