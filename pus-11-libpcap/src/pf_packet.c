/*
 * Data:                2009-06-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc pf_packet.c -o pf_packet
 * Uruchamianie:        $ ./pf_packet
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <errno.h>

int main(int argc, char **argv) {

    int                     sockfd, bytes;
    struct sockaddr_ll      sockaddr;
    socklen_t               sockaddr_len;
    unsigned char           buf[4096];

    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        sockaddr_len = sizeof(struct sockaddr_ll);
        bytes = recvfrom(sockfd, buf, 4096, 0, (struct sockaddr*)&sockaddr, &sockaddr_len);
        if (bytes == -1) {
            perror("recvfrom()");
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "Packet received: %u bytes\n", bytes);

        switch (sockaddr.sll_family) {
        case PF_PACKET:
            fprintf(stdout, "sa_family: PF_PACKET\n");
            break;
        default:
            fprintf(stdout, "sa_family: OTHER\n");
        }

        switch (ntohs(sockaddr.sll_protocol)) {
        case ETH_P_ARP:
            fprintf(stdout, "sll_protocol: ETH_P_ARP\n");
            break;
        case ETH_P_IP:
            fprintf(stdout, "sll_protocol: ETH_P_IP\n");
            break;
        case ETH_P_LOOP:
            fprintf(stdout, "sll_protocol: ETH_P_LOOP\n");
            break;
        default:
            fprintf(stdout, "sll_protocol: OTHER\n");
        }

        fprintf(stdout, "interface index: %u\n", sockaddr.sll_ifindex);

        switch (sockaddr.sll_hatype) {
        case ARPHRD_ETHER:
            fprintf(stdout, "sll_hatype: ARPHRD_ETHER\n");
            break;
        case ARPHRD_LOOPBACK:
            fprintf(stdout, "sll_hatype: ARPHRD_LOOPBACK\n");
            break;
        default:
            fprintf(stdout, "sll_hatype: OTHER\n");
        }

        switch (sockaddr.sll_pkttype) {
        case PACKET_HOST:
            fprintf(stdout, "sll_pkttype: PACKET_HOST\n");
            break;
        case PACKET_BROADCAST:
            fprintf(stdout, "sll_pkttype: PACKET_BROADCAST\n");
            break;
        case PACKET_MULTICAST:
            fprintf(stdout, "sll_pkttype: PACKET_MULTICAST\n");
            break;
        case PACKET_OTHERHOST:
            fprintf(stdout, "sll_pkttype: PACKET_OTHERHOST\n");
            break;
        case PACKET_OUTGOING:
            fprintf(stdout, "sll_pkttype: PACKET_OUTGOING\n");
            break;
        case PACKET_LOOPBACK:
            fprintf(stdout, "sll_pkttype: PACKET_LOOPBACK\n");
            break;
        default:
            fprintf(stdout, "sll_pkttype: OTHER\n");
        }

        fprintf(stdout, "mac: %s\n", ether_ntoa((struct ether_addr*)sockaddr.sll_addr));
        fprintf(stdout, "\n");
    }

    exit(EXIT_SUCCESS);
}
