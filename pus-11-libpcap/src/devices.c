/*
 * Data:                2009-06-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc devices.c -lpcap -o devices
 * Uruchamianie:        $ ./devices
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <sys/socket.h>         /* AF_INET, ... */
#include <netdb.h>              /* getnameinfo() */
#include <netinet/ether.h>      /* ether_ntoa() */
#include <netpacket/packet.h>   /* struct sockaddr_ll */
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>


int main(int argc, char **argv) {

    int                     err;
    char                    errbuf[PCAP_ERRBUF_SIZE];

    /* Wskaznik na pierwszy element listy interfejsow: */
    pcap_if_t               *alldevsp;
    /* Wskaznik na interfejs: */
    pcap_if_t               *if_t;
    /* Wskaznik na strukture adresowa interfejsu: */
    pcap_addr_t             *addr_t;

    /* Bufor dla adresow IP w postaci czytelnej dla czlowieka: */
    char                    buf[NI_MAXHOST];

    sa_family_t             family; /* Rodzina adresowa */

    /* Wskaznik na strukture adresowa (dla adresow warstwy lacza danych): */
    struct sockaddr_ll      *addr_l;
    socklen_t               addr_len; /* Rozmiar struktury adresowej */

    /*
     * Zwraca liste interfejsow sieciowych, ktore moga zostac uzyte do
     * przechwytywania pakietow.
     * Wymagane uprawnienie CAP_NET_RAW (uprawnienia roota).
     */
    if (pcap_findalldevs(&alldevsp, errbuf) == -1) {
        fprintf(stderr, "%s", errbuf);
        exit(EXIT_FAILURE);
    }

    /*
     * Wypisanie listy interfejsow sieciowych.
     */
    for (if_t = alldevsp; if_t; if_t = if_t->next) {

        /* Nazwa i opis interfejsu: */
        fprintf(stdout, "Name: %s\n", if_t->name);
        fprintf(stdout, "Description: %s\n", if_t->description);

        /* Lista adresow IP oraz adresow MAC nalezacych do danego interfejsu: */
        for (addr_t = if_t->addresses; addr_t; addr_t = addr_t->next) {

            family = addr_t->addr->sa_family;

            if (family == AF_INET || family == AF_INET6) {

                /*
                 * Zapamietanie rozmiaru struktury adresowej w
                 * zaleznosci od rodziny adresowej:
                 */
                if (family == AF_INET) {
                    addr_len = sizeof(struct sockaddr_in);
                } else {
                    addr_len = sizeof(struct sockaddr_in6);
                }

                /*
                 * Przeksztalcenie adresu do postaci zrozumialej dla
                 * czlowieka:
                 */
                err = getnameinfo(addr_t->addr, addr_len,
                                  buf, NI_MAXHOST, NULL, 0,
                                  NI_NUMERICHOST | NI_NUMERICSERV);

                if (err != 0) {
                    fprintf(stderr, "getnameinfo(): %s\n",
                            gai_strerror(err));

                    exit(EXIT_FAILURE);
                }
                fprintf(stdout, "IP address getnameinfo: %s\n", buf);


                if(family == AF_INET) {
                    memset(buf, 0, NI_MAXHOST);
                    inet_ntop(AF_INET, &(((struct sockaddr_in *) (addr_t->addr))->sin_addr), buf, NI_MAXHOST);

                } else {
                    memset(buf, 0, NI_MAXHOST);
                    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) (addr_t->addr))->sin6_addr), buf, NI_MAXHOST);
                }


                fprintf(stdout, "IP address ntop:        %s\n", buf);

            } else if (family == AF_PACKET) {

                addr_l = (struct sockaddr_ll*)addr_t->addr;
                /* Wypisanie adresu MAC: */
                if ((addr_l->sll_hatype == ARPHRD_ETHER)
                        && (addr_l->sll_halen == 6)) {
                    fprintf(stdout, "MAC address: %s\n",
                            ether_ntoa((struct ether_addr*)addr_l->sll_addr));
                }
            }
        }

        fprintf(stdout, "\n");
    }

    /* Zwolnienie listy intrfejsow: */
    pcap_freealldevs(alldevsp);

    exit(EXIT_SUCCESS);
}
