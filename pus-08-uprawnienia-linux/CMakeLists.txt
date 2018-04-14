cmake_minimum_required(VERSION 3.9)
project(pus-08-uprawnienia-linux)

add_executable(zad01-suidserver src/suidserver.c)
add_executable(zad02-rootserver src/rootserver.c)
add_executable(zad03-daemon src/daemon.c)
add_executable(zad04-fserver src/fserver.c)
add_executable(zad05-capserver src/capserver.c)
target_link_libraries(zad05-capserver cap)