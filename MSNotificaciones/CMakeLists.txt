cmake_minimum_required(VERSION 3.16)




set(CMAKE_CXX_STANDARD 17)



add_executable(MSNotificaciones
  main.cpp
)


find_package(Boost REQUIRED)
target_include_directories(MSNotificaciones PRIVATE /usr/local/include)
target_link_libraries(MSNotificaciones PRIVATE Boost::boost pthread)
