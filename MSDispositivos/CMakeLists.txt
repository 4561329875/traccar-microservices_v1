cmake_minimum_required(VERSION 3.16)






set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})
find_package(nlohmann_json  REQUIRED)

enable_language(C)

add_executable(MicorsevicioDispocitivosDrivers
    conexionPostrgrest.c
    obtenerTodosDispositivos.c
    obtenerDispositivosCondicion.c
    main.cpp
    obtenerTodosDispositivos.h
    obtenerDispositivosCondicion.h
    ObteDisConUni.c
    ObteDisConUni.h
    GuardarDispositivo.c
    GuardarDispositivo.h
    ActualizarDispositivo.c
    ActualizarDispositivo.h
    EliminarDispositivo.c
    EliminarDispositivo.h
    ObtenerTodosConductores.c
    ObtenerTodosConductores.h
    ObtenerCondiciConductores.c
    ObtenerCondiciConductores.h
    ObtenerCondiciUniConductores.c
    ObtenerCondiciUniConductores.h
    GuardarConductor.c
    GuardarConductor.h
    ActualizarConductor.c
    ActualizarConductor.h
    EliminarConductor.c
    EliminarConductor.h
    ActualizarAcumuladores.c
    ActualizarAcumuladores.h
    ManejadorDeIMG.c
    ManejadorDeIMG.h
    benchmark.cpp
    benchmark.h

    metrics_logger.h
    metrics_logger.cpp

    ConversorJSONDispositivos.c
    ConversorJSONDispositivos.h
    
)

target_link_libraries(MicorsevicioDispocitivosDrivers ${PostgreSQL_LIBRARIES})
target_link_libraries(MicorsevicioDispocitivosDrivers nlohmann_json)

include(GNUInstallDirs)
install(TARGETS MicorsevicioDispocitivosDrivers
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
