/*
 * Data:                2009-06-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc icmp_sniff.c -lpcap -o icmp_sniff
 * Uruchamianie:        $ ./icmp_sniff <INTERFACE NAME>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <net/ethernet.h> /* struct ether_header, ETHERTYPE_IP */
#include <netinet/ip.h> /* struct iphdr */
#include <netinet/in.h> /* IP protocols (IPPROTO_ICMP) */
#include <netinet/ip_icmp.h> /* struct icmphdr */
#include <arpa/inet.h> /* inet_ntop() */
#include <signal.h> /* sigaction() */
#include <string.h> /* memset() */
#include <errno.h>

/* Deskryptor wykorzystywany do przechwytywania pakietow: */
pcap_t                  *descriptor;

/*
 * Struktura z informacjami na temat liczby
 * przechwyconych/porzuconych pakietow:
 */
struct pcap_stat        stats;

/* Funkcja wykorzystywana do obslugi sygnalu SIGINT (CTRL + C): */
void handler(int sig) {

    /* Pobranie informacji statystycznych: */
    if (pcap_stats(descriptor, &stats) == -1) {
        pcap_perror(descriptor, "pcap_stats()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "\nReceived: %u, dropped: %u\n",
            stats.ps_recv, stats.ps_drop);

    exit(EXIT_SUCCESS);
}


int main(int argc, char **argv) {

    int                     err;
    char                    errbuf[PCAP_ERRBUF_SIZE];

    /*
     * Wskaznik do struktury przechowujacej metadane
     * na temat przechwyconego pakietu:
     */
    struct pcap_pkthdr      *pkt_header;

    /* Wskaznik na dane przechwyconego pakietu: */
    const unsigned char     *pkt_data;

    /* Typ warstwy lacza danych: */
    int                     data_link_type;

    /* Wskaznik na pierwszy bajt naglowka ramki Ethernet: */
    struct ether_header     *ether_header;
    /* Wskaznik na pierwszy bajt naglowka IP: */
    struct iphdr            *ip_header;
    /* Wskaznik na pierwszy bajt naglowka ICMP: */
    struct icmphdr          *icmp_header;

    /* Struktura okreslajaca dyspozycje dla sygnalu SIGINT: */
    struct sigaction        action;

    /* Adres IPv4: */
    char                    address[INET_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie deskryptora do przechwytywania pakietow: */
    descriptor = pcap_open_live(argv[1], 65565, 0, 10, errbuf);
    if (descriptor == NULL) {
        fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    memset(&action, 0, sizeof(struct sigaction)); /* Wyzerowanie struktury */
    action.sa_handler = handler; /* Okreslenie funkcji obslugujacej sygnal */

    /* Okreslenie dyspozycji dla sygnalu SIGINT (CTRL + C): */
    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("sigaction() failed!\n");
        exit(EXIT_FAILURE);
    }

    /* Sprawdzenie typu warstwy lacza danych: */
    data_link_type = pcap_datalink(descriptor);
    if (data_link_type != DLT_EN10MB) {
        /* Jezeli inny od ETHERNET: */
        exit(EXIT_FAILURE);
    }

    for (;;) {

        /* Odczytanie przechwyconego pakietu: */
        err = pcap_next_ex(descriptor, &pkt_header, &pkt_data);
        if (err < 0) {
            pcap_perror(descriptor, "pcap_next_ex()");
            exit(EXIT_FAILURE);
        } else if (err == 0) {
            continue;
        }

        /*
         * Jezeli rozmiar jest mniejszy od:
         * rozmiar naglowka ramki Ethernet
         * + min. rozmiar naglowka IP (20  bajtow)
         * Innymi slowy upewniamy sie czy istnieje naglowek IP.
         */
        if (pkt_header->caplen < ETHER_HDR_LEN + 20) {
            fprintf(stderr, "Packet length < ETHER_HDR_LEN + 20\n");
            continue;
        }

        /* Wskazni na pierwszy bajt ramki Ethernet: */
        ether_header = (struct ether_header*)pkt_data;

        /* Interesuja nas tylko ramki przenoszace pakiety IP: */
        if (ntohs(ether_header->ether_type) != ETHERTYPE_IP) {
            continue;
        }

        /* Wskaznik na pierwszy bajt naglowka IP: */
        ip_header = (struct iphdr*)((unsigned char*)ether_header
                                    + ETHER_HDR_LEN);

        /* Tylko protokol ICMP: */
        if (ip_header->protocol != IPPROTO_ICMP) {
            continue;
        }

        /*
         * Jezeli rozmiar przechwyconego pakietu jest mniejszy od:
         * rozmiar naglowka ramki Ethernet
         * + rozmiar calego pakietu IP (z danymi)
         */
        if (pkt_header->caplen < ETHER_HDR_LEN + ntohs(ip_header->tot_len)) {
            fprintf(stderr, "Packet length < ETHER_HDR_LEN "
                    "+ ntohs(ip_header->tot_len)\n");
            continue;
        }

        /* Wskaznik na pierwszy bajt naglowka ICMP: */
        icmp_header = (struct icmphdr*)
                      ((unsigned char*)ip_header + (ip_header->ihl * 4));

        /* Wypisanie informacji na temat komunikatu ICMP Echo: */
        if ((icmp_header->type == 8) && (icmp_header->code == 0)) {

            /* Konwersja IP do postaci tekstowej: */
            if (inet_ntop(AF_INET, &ip_header->saddr,
                          address, INET_ADDRSTRLEN) == NULL) {
                perror("inet_ntop()");
                exit(EXIT_FAILURE);
            }

            fprintf(stdout, "Received ICMP Echo [ID: %u, SEQ: %u, FROM: %s]\n",
                    ntohs(icmp_header->un.echo.id),
                    ntohs(icmp_header->un.echo.sequence),
                    address);
        }
    }

    exit(EXIT_SUCCESS);
}
