cmake_minimum_required(VERSION 3.16)




set(CMAKE_CXX_STANDARD 17)



add_executable(MSUsuarios
  main.cpp
)


find_package(Boost REQUIRED)
target_include_directories(MSUsuarios PRIVATE /usr/local/include)
target_link_libraries(MSUsuarios PRIVATE Boost::boost pthread)
