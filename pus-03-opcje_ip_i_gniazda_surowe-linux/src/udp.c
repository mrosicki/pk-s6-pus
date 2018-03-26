/*
 * Data:                2009-02-27
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc udp.c -o udp
 * Uruchamianie:        $ ./udp <adres IP lub nazwa domenowa> <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef __USE_BSD
#define __USE_BSD
#endif

#include <netinet/ip.h>

#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include "checksum.h"

#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "192.0.2.1"

/* Struktura pseudo-naglowka (do obliczania sumy kontrolnej naglowka UDP): */
struct phdr {
    struct in_addr ip_src, ip_dst;
    unsigned char  unused;
    unsigned char  protocol;
    unsigned short length;

};

int main(int argc, char **argv) {

    int sockfd; /* Deskryptor gniazda. */
    int socket_option; /* Do ustawiania opcji gniazda. */
    int retval; /* Wartosc zwracana przez funkcje. */

    /* Struktura zawierajaca wskazowki dla funkcji getaddrinfo(): */
    struct addrinfo hints;

    /*
     * Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do
     * poruszania sie po elementach listy:
     */
    struct addrinfo *rp, *result;

    /* Zmienna wykorzystywana do obliczenia sumy kontrolnej: */
    unsigned short checksum;

    /* Bufor na naglowek IP, naglowek UDP oraz pseudo-naglowek: */
    unsigned char datagram[sizeof(struct ip) + sizeof(struct udphdr)
                           + sizeof(struct phdr)] = {0};

    /* Wskaznik na naglowek IP (w buforze okreslonym przez 'datagram'): */
    struct ip *ip_header = (struct ip *) datagram;

    /* Wskaznik na naglowek UDP (w buforze okreslonym przez 'datagram'): */
    struct udphdr *udp_header    = (struct udphdr *)
            (datagram + sizeof(struct ip));
    /* Wskaznik na pseudo-naglowek (w buforze okreslonym przez 'datagram'): */
    struct phdr   *pseudo_header = (struct phdr *)
            (datagram + sizeof(struct ip)
             + sizeof(struct udphdr));
    /* SPrawdzenie argumentow wywolania: */
    if (argc != 3) {
        fprintf(
                stderr,
                "Invocation: %s <HOSTNAME OR IP ADDRESS> <PORT>\n",
                argv[0]
        );

        exit(EXIT_FAILURE);
    }

    /* Wskazowki dla getaddrinfo(): */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_INET; /* Domena komunikacyjna (IPv4). */
    hints.ai_socktype = SOCK_RAW; /* Typ gniazda. */
    hints.ai_protocol = IPPROTO_UDP; /* Protokol. */


    retval = getaddrinfo(argv[1], NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    /* Opcja okreslona w wywolaniu setsockopt() zostanie wlaczona: */
    socket_option = 1;

    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /* Utworzenie gniazda dla protokolu UDP: */
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            perror("socket()");
            continue;
        }

        /* Ustawienie opcji IP_HDRINCL: */
        retval = setsockopt(
                sockfd,
                IPPROTO_IP, IP_HDRINCL,
                &socket_option, sizeof(int)
        );

        if (retval == -1) {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        } else {
            /* Jezeli gniazdo zostalo poprawnie utworzone i
             * opcja IP_HDRINCL ustawiona: */
            break;
        }
    }

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Client failure: could not create socket.\n");
        exit(EXIT_FAILURE);
    }

    /********************************/
    /* Wypelnienie pol naglowka IP: */
    /********************************/
    ip_header->ip_hl  = 5; /* 5 * 32 bity = 20 bajtow */
    ip_header->ip_v   = 4; /* Wersja protokolu (IPv4). */
    ip_header->ip_tos = 0; /* Pole TOS wyzerowane. */

    /* Dlugosc (naglowek IP + dane). Wg MAN: "Always filled in": */
    ip_header->ip_len = sizeof(struct ip)
                        + sizeof(struct udphdr);

    /* Wypelniane przez jadro systemu, jezeli podana wartosc to zero: */
    ip_header->ip_id = 0; /* Pole Identification. */

    ip_header->ip_off = 0; /* Pole Fragment Offset. */
    ip_header->ip_ttl = 255; /* TTL */

    /* Identyfikator enkapsulowanego protokolu: */
    ip_header->ip_p = IPPROTO_UDP;

    /* Adres zrodlowy ("Filled in when zero"): */
    ip_header->ip_src.s_addr = inet_addr(SOURCE_ADDRESS);

    /* Adres docelowy (z argumentu wywolania programu): */
    ip_header->ip_dst.s_addr = ((struct sockaddr_in *) rp->ai_addr)
            ->sin_addr.s_addr;

    /* Suma kontrolna naglowka IP - "Always filled in":
     *
     * ip_header->ip_sum            =       internet_checksum(
     *                                              (unsigned short *)ip_header,
     *                                              sizeof(struct ip)
     *                                              );
     */

    /*********************************/
    /* Wypelnienie pol naglowka UDP: */
    /*********************************/

    /* Port zrodlowy: */
    udp_header->uh_sport = htons(SOURCE_PORT);
    /* Port docelowy (z argumentu wywolania): */
    udp_header->uh_dport = htons(atoi(argv[2]));

    /* Rozmiar naglowka UDP i danych. W tym przypadku tylko naglowka: */
    udp_header->uh_ulen = htons(sizeof(struct udphdr));

    /************************************/
    /* Wypelnienie pol pseudo-naglowka: */
    /************************************/

    /* Zrodlowy adres IP: */
    pseudo_header->ip_src.s_addr = ip_header->ip_src.s_addr;
    /* Docelowy adres IP: */
    pseudo_header->ip_dst.s_addr = ip_header->ip_dst.s_addr;
    /* Pole wyzerowane: */
    pseudo_header->unused        = 0;
    /* Identyfikator enkapsulowanego protokolu: */
    pseudo_header->protocol      = ip_header->ip_p;
    /* Rozmiar naglowka UDP i danych: */
    pseudo_header->length        = udp_header->uh_ulen;
    /* Obliczenie sumy kontrolnej na podstawie naglowka UDP i pseudo-naglowka: */
    udp_header->uh_sum           = 0;
    checksum = internet_checksum(
            (unsigned short *) udp_header,
            sizeof(struct udphdr)
            + sizeof(struct phdr)
    );

    udp_header->uh_sum = (checksum == 0) ? 0xffff : checksum;

    fprintf(stdout, "Sending UDP...\n");

    /* Wysylanie datagramow co 1 sekunde: */
    for (;;) {

        /*
         * Prosze zauwazyc, ze pseudo-naglowek nie jest wysylany
         * (ale jest umieszczony w buforze za naglowkiem UDP dla wygodnego
         * obliczania sumy kontrolnej):
         */
        retval = sendto(
                sockfd,
                datagram, ip_header->ip_len,
                0,
                rp->ai_addr, rp->ai_addrlen
        );

        if (retval == -1) {
            perror("sendto()");
        }

        sleep(1);
    }

    exit(EXIT_SUCCESS);
}
