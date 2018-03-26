/*
 * Data:                2009-02-18
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc ipaddr.c -o ipaddr
 * Uruchamianie:        $ ./ipaddr <adres IPv4 lub IPv6>
 */

#include <sys/socket.h>
#include <netinet/in.h> /* INET_ADDRSTRLEN, INET6_ADDRSTRLEN */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

char* ipv4_binary_string(uint32_t addr);

int main(int argc, char** argv) {

    int                     i;
    int                     retval; /* Wartosc zwracana przez funkcje. */
    struct sockaddr_in      addr4;  /* Struktura adresowa dla IPv4. */
    struct sockaddr_in6     addr6;  /* Struktura adresowa dla IPv6. */

    /* Bufory na adresy w postaci czytelnej dla czlowieka (tekstowej): */
    char                    buff4[INET_ADDRSTRLEN];
    char                    buff6[INET6_ADDRSTRLEN];

    /*
     * Flagi, ktore sluza do oznaczenia, ze konwersja adresu IPv4 lub IPv6
     * nie powiodla sie:
     */
    int                     invalid4, invalid6;

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <IPv4 OR IPv6 ADDRESS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Stan poczatkowy: (bez bledow konwersji adresow IP) */
    invalid4 = 0;
    invalid6 = 0;

    /******************************************************************************/
    /*             Konwersja z postaci czytelnej dla czlowieka                    */
    /*                inet_aton(), inet_addr(), inet_pton()                       */
    /******************************************************************************/
    fprintf(
        stdout,
        "Internet host address to network address structure conversion:\n\n"
    );

    /*
     * Funkcja inet_aton() przeprowadza konwersje adresu IPv4 z postaci
     * kropkowo-dziesietnej (pierwszy parametr) do postaci zrozumialej dla maszyny
     * (drugi parametr). Drugim parametrem jest wskaznik na strukture in_addr.
     */
    if (inet_aton(argv[1], &(addr4.sin_addr)) == 0) {
        fprintf(stderr, "inet_aton(): invalid IPv4 address!\n");
        invalid4 = 1; /* Blad konwersji adresu IPv4. */
    } else {
        fprintf(
            stdout, "inet_aton(): %s (binary)\n",
            ipv4_binary_string(addr4.sin_addr.s_addr)
        );
    }


    /*
     * Funkcja inet_addr() przeprowadza konwersje adresu IPv4 z postaci
     * kropkowo-dziesietnej (parametr funkcji) do postaci zrozumialej dla maszyny
     * (wartosc zwracana). Funkcja inet_addr() zwraca wartosc typu in_addr_t.
     *
     * <netinet/in.h>:
     * typedef uint32_t in_addr_t; *
     */
    if ((addr4.sin_addr.s_addr = inet_addr(argv[1])) == INADDR_NONE) {
        fprintf(stderr, "inet_addr(): invalid IPv4 address!\n");
        invalid4 = 1;
    } else {
        fprintf(
            stdout, "inet_addr(): %s (binary)\n",
            ipv4_binary_string(addr4.sin_addr.s_addr)
        );
    }

    /*
     * Funkcja inet_pton() przeprowadza konwersje adresu IPv4 lub IPv6 (pierwszy
     * parametr) z postaci tekstowej (drugi parametr) do postaci zrozumialej
     * dla maszyny (trzeci parametr).
     *
     * Dla IPv4:
     */
    retval = inet_pton(AF_INET, argv[1], &addr4.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid IPv4 address!\n");
        invalid4 = 1;
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    } else {
        fprintf(
            stdout, "inet_pton(): %s (binary)\n",
            ipv4_binary_string(addr4.sin_addr.s_addr)
        );
    }

    /* Dla IPv6: */
    retval = inet_pton(AF_INET6, argv[1], &addr6.sin6_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid IPv6 address!\n");
        invalid6 = 1;
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    } else {
        /* Wypisanie adresu IPv6 (format heksadecymalny): */
        fprintf(stdout, "inet_pton(): ");
        i = 0;
        for (;;) {
            /* Wypisanie bajtu (2 cyfr szesnastkowych): */
            fprintf(stdout, "%.2x", addr6.sin6_addr.s6_addr[i]);

            ++i;
            if (i == 16) {
                break;
            }

            if (i%2 == 0) { /* Co 4 cyfry heksadecymalne (2 bajty). */
                fprintf(stdout, ":");
            }

        }
        fprintf(stdout, " (hex)\n");
    }

    /* Jezeli podany tekst nie moze zostac zaklasyfikowany
     * jako poprawny adres IP: */
    if (invalid4 & invalid6) {
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /*             Konwersja do postaci czytelnej dla czlowieka                   */
    /*                     inet_ntoa(), inet_ntop()                               */
    /******************************************************************************/
    fprintf(
        stdout,
        "\nNetwork address structure to internet host address conversion:\n\n"
    );

    /*
     * Jezeli podany w argumencie argv[1] string jest adresem IPv4
     * (przeszedl poprawna konwersje z postaci czytelnej dla czlowieka):
     */
    if (invalid4 != 1) {

        /*
         * Funkcja inet_ntoa() przeprowadza konwersje adresu IPv4 z postaci
         * zrozumialej dla maszyny do postaci tekstowej (wartosc zwracana).
         * String jest zwracany w statycznie zaalokowanym buforze. Kolejne wywolanie
         * funkcji spowoduje jego nadpisanie.
         */
        fprintf(stdout, "inet_ntoa(): %s\n", inet_ntoa(addr4.sin_addr));

        /*
         * Funkcja inet_ntop() przeprowadza konwersje adresu IPv4 lub IPv6 (pierwszy
         * parametr) z postaci zrozumialej dla maszyny (drugi parametr) do postaci
         * tekstowej (trzeci parametr). Ostatnim parametrem jest rozmiar bufora dla
         * adresu w formie tekstowej.
         *
         * Dla IPv4:
         */
        fprintf(
            stdout, "inet_ntop(): %s\n",
            inet_ntop(AF_INET, &(addr4.sin_addr), buff4, INET_ADDRSTRLEN)
        );
    }

    /*
     * Jezeli podany w argumencie argv[1] string jest adresem IPv6
     * (przeszedl poprawna konwersje z postaci czytelnej dla czlowieka):
     */
    if (invalid6 != 1) {
        /* Wywolanie funkcji inet_ntop() dla IPv6: */
        fprintf(
            stdout, "inet_ntop(): %s\n",
            inet_ntop(AF_INET6, &(addr6.sin6_addr), buff6, INET6_ADDRSTRLEN)
        );
    }

    exit(EXIT_SUCCESS);
}


/*
 * Funkja odpowiedzialna za konwersje adresu IPv4 z postaci zrozumialej dla
 * maszyny do stringu reprezentujacego adres w formie binarnej.
 */
char* ipv4_binary_string(uint32_t addr) {
    int i, j;
    static char buff[36];
    const uint32_t pos = 0x80000000;
    addr = ntohl(addr);

    for (i=1, j=0; i < 32; ++i, ++j) {
        buff[j] = (pos & addr) ? '1' : '0';
        addr <<= 1;
        if (i%8 == 0) {
            buff[++j] = '.';
        }
    }

    buff[j++] = (pos & addr) ? '1' : '0';
    buff[j] = '\0';
    return buff;
}

/*
CASE STUDY:
./ip2addr 255.255.255.255
./ip2addr abc
./ip2addr 192.168.1
./ip2addr 999.1.2
./ip2addr 1.999.2
./ip2addr 1.2.999
./ip2addr ::1
*/
