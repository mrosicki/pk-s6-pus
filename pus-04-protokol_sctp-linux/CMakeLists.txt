cmake_minimum_required(VERSION 3.9)
project(pus-04-protokol_sctp-linux)

add_executable(zad01-client src/client1.c)
add_executable(zad01-server src/server1.c)

add_executable(zad02-client src/client2.c)
target_link_libraries(zad02-client sctp)
add_executable(zad02-server src/server2.c)
target_link_libraries(zad02-server sctp)

add_executable(zad03-client src/client3.c)
target_link_libraries(zad03-client sctp)
add_executable(zad03-server src/server3.c)
target_link_libraries(zad03-server sctp)


