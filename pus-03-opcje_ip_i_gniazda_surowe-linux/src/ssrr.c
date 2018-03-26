/*
 * Data:                2009-02-27
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc ssrr.c -o ssrr
 * Uruchamianie:        $ ./ssrr <adres IP lub nazwa domenowa>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "checksum.h"

int main(int argc, char** argv) {

    /* Naglowek ICMP: */
    struct icmphdr          icmp_header = {0};

    /* Struktura zawierajaca wskazowki dla funkcji getaddrinfo(): */
    struct addrinfo         hints;

    /*
     * Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do
     * poruszania sie po elementach listy:
     */
    struct addrinfo         *rp, *result;

    int                     sockfd; /* Deskryptor gniazda. */
    int                     retval; /* Wartosc zwracana przez funkcje. */

    /*
     * Opcje IP:
     * NOP, SSRR, len, ptr, adr_IP_1, adr_IP_2, IP_docelowe */
    unsigned char ip_options[16] = {
        1, 0x89, 15, 4,
        192,0,2,1,
        195,136,186,1,
        213,172,178,41
    };

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <HOSTNAME OR IP ADDRESS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Wskazowki dla getaddrinfo(): */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         =       AF_INET; /* Domena komunikacyjna (IPv4). */
    hints.ai_socktype       =       SOCK_RAW; /* Typ gniazda. */
    hints.ai_protocol       =       IPPROTO_ICMP; /* Protokol. */


    /* Pierwszy argument to adres IP lub nazwa domenowa: */
    retval = getaddrinfo(argv[1], NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /* Utworzenie gniazda dla protokolu ICMP: */
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            perror("socket()");
            continue;
        }

        /* Ustawienie opcji IP: */
        retval = setsockopt(
                     sockfd, IPPROTO_IP, IP_OPTIONS,
                     (void*)ip_options, sizeof(ip_options)
                 );

        if (retval == -1) {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        } else {
            /* Jezeli gniazdo zostalo poprawnie utworzone i
             * opcje IP ustawione: */
            break;
        }
    }

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Client failure: could not create socket.\n");
        exit(EXIT_FAILURE);
    }

    /* Wypelnienie pol naglowka ICMP Echo: */
    srand(time(NULL));
    /* Typ komunikatu: */
    icmp_header.type                =       ICMP_ECHO;
    /* Kod komunikatu: */
    icmp_header.code                =       0;
    /* Identyfikator: */
    icmp_header.un.echo.id          =       htons(getpid());
    /* Numer sekwencyjny: */
    icmp_header.un.echo.sequence    =       htons((unsigned short)rand());
    /* Suma kontrolna (plik checksum.h): */
    icmp_header.checksum            =       internet_checksum(
                                                (unsigned short *)&icmp_header,
                                                sizeof(icmp_header)
                                            );

    fprintf(stdout, "Sending ICMP Echo...\n");
    /* Wyslanie komunikatu ICMP Echo: */
    retval = sendto(
                 sockfd,
                 &icmp_header, sizeof(icmp_header),
                 0,
                 rp->ai_addr, rp->ai_addrlen
             );

    if (retval == -1) {
        perror("sentdo()");
    }

    /* Zwalniamy liste zaalokowana przez funkcje getaddrinfo(): */
    freeaddrinfo(result);

    close(sockfd);
    exit(EXIT_SUCCESS);
}
