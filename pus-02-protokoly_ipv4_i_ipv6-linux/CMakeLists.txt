cmake_minimum_required(VERSION 3.9)
project(pus-02-protokoly_ipv4_i_ipv6-linux)


add_executable(zad01_ipaddr src/ipaddr.c)
add_executable(zad02_hostname2ip src/hostname2ip.c)
add_executable(zad03_client src/client4.c)
add_executable(zad03_server src/server6.c)
add_executable(zad04_client src/client6.c)
add_executable(zad04_server src/server4.c)
add_executable(zad05_client src/client.c)
add_executable(zad05_server src/server.c)

