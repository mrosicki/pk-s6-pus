cmake_minimum_required(VERSION 3.9)
project(pus-07-netfilter)

#include_directories(/usr/src/linux-headers-4.13.0-37/include)
#link_directories(/usr/src/linux-headers-4.13.0-37/lib)

add_executable(zad01-queue src/queue.c)
target_link_libraries(zad01-queue lnfnetlink netfilter_queue)

add_executable(zad02-queuemod src/queuemod.c)
target_link_libraries(zad02-queuemod netfilter_queue)

add_executable(zad03-iptc src/iptc.c)
target_link_libraries(zad03-iptc ip4tc)

add_executable(zad04-iptcmod src/iptcmod.c)
target_link_libraries(zad04-iptcmod iptc )