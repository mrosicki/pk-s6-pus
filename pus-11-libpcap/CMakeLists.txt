cmake_minimum_required(VERSION 3.9)
project(pus-11-libpcap)


add_executable(zad01-devices src/devices.c)
target_link_libraries(zad01-devices pcap)

add_executable(zad02-icmp_sniffer src/icmp_sniff.c)
target_link_libraries(zad02-icmp_sniffer pcap)

add_executable(zad03-icmp_filter src/icmp_filter.c)
target_link_libraries(zad03-icmp_filter pcap)

add_executable(zad04-arp_sniffer src/arp_sniff.c)
target_link_libraries(zad04-arp_sniffer pcap)

add_executable(zad05-arp_spoof src/arp_spoof.c)
target_link_libraries(zad05-arp_spoof pcap)
