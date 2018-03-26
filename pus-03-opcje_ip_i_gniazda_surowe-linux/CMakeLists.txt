cmake_minimum_required(VERSION 3.9)
project(pus-03-opcje_ip_i_gniazda_surowe-linux)


add_executable(zad_01_ssrr src/ssrr.c src/checksum.h)
add_executable(zad_02_udp src/udp.c src/checksum.h)
add_executable(zad_03_tcp src/tcp.c src/checksum.h)
add_executable(zad_04_udpv6 src/udpv6.c)
add_executable(zad_05_ping src/ping.c src/checksum.h)


