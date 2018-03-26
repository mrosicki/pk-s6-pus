/*
 * Data:                2009-02-18
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc hostname2ip.c -o hostname2ip
 * Uruchamianie:        $ ./hostname2ip <nazwa domenowa>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h> /* getaddrinfo() */


int main(int argc, char** argv) {

    /* Struktura zawierajaca wskazowki dla funkcji getaddrinfo(): */
    struct addrinfo         hints;

    /*
     * Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do
     * poruszania sie po elementach listy:
     */
    struct addrinfo         *result, *rp;

    /* Wartosc zwracana przez funkcje: */
    int                     retval;

    /* Zmienna przechowuje wersje protokolu IP (4 lub 6): */
    int                     ip_version;

    /*
     * Adres IPv4 lub IPv6 w postaci tekstowej. Rozmiar nie powinien przekroczyc
     * INET6_ADDRSTRLEN. Pomimo tego faktu korzystamy z NI_MAXHOST
     * (okresla maksymalny rozmiar bufora dla nazwy domenowej lub adresu IP
     * w formie tekstowej.
     */
    char                    address[NI_MAXHOST];

    /* Rozmiar gniazdowej struktury adresowej: */
    socklen_t               sockaddr_size;

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <HOSTNAME>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    /* Pozwalamy na AF_INET or AF_INET6: */
    hints.ai_family         =       AF_UNSPEC;
    /* Gniazdo typu SOCK_STREAM (TCP): */
    hints.ai_socktype       =       SOCK_STREAM;
    /* Dowolny protokol: */
    hints.ai_protocol       =       0;

    if ((retval = getaddrinfo(argv[1], NULL, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }


    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /* Zapamietujemy wersje protokolu oraz rozmiar struktury adresowej: */
        if (rp->ai_family == AF_INET) { /* IPv4 */
            ip_version = 4;
            sockaddr_size = sizeof(struct sockaddr_in);
        } else { /* IPv6 */
            ip_version = 6;
            sockaddr_size = sizeof(struct sockaddr_in6);
        }

        /*
         * Konwersja adresu z postaci zrozumialej dla maszyny
         * do postaci tekstowej:
         */
        retval = getnameinfo(
                     (struct sockaddr *)rp->ai_addr,
                     sockaddr_size,
                     address, NI_MAXHOST,
                     NULL, 0,
                     NI_NUMERICHOST
                 );

        if (retval != 0) {
            fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(retval));
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "IPv%d: %s\n", ip_version, address);
    }

    /* Zwalniamy liste zaalokowana przez funkcje getaddrinfo(): */
    if (result != NULL) {
        freeaddrinfo(result);
    }

    exit(EXIT_SUCCESS);
}
