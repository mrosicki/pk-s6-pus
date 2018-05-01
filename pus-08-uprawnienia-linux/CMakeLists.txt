cmake_minimum_required(VERSION 3.9)
project(pus-08-uprawnienia-linux)

add_executable(zad01_suidserver src/suidserver.c)
add_executable(zad02_rootserver src/rootserver.c)
add_executable(zad03_daemon src/daemon.c)
add_executable(zad04_fserver src/fserver.c)
add_executable(zad05_capserver src/capserver.c)
target_link_libraries(zad05_capserver cap)
