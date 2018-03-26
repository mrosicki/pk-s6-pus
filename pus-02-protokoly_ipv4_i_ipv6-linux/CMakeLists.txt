cmake_minimum_required(VERSION 3.9)
project(pus-02-protokoly_ipv4_i_ipv6-linux)


add_executable(zad01_ipaddr src/ipaddr.c)
add_executable(zad02_hostname2ip src/hostname2ip.c)
add_executable(zad03_client src/zad03_client_ipv4.c)
add_executable(zad03_server src/zad03_server_ipv6.c)
add_executable(zad04_client src/zad04_client_ipv6.c)
add_executable(zad04_server src/zad04_server_ipv4.cpp)
add_executable(zad05_client src/zad05_client.c)
add_executable(zad05_server src/zad05_server.c)

