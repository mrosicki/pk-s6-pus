#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "checksum.h"

#define MESSAGE_SIZE 30

size_t icmp_datagram_size = sizeof(struct icmphdr) + MESSAGE_SIZE;
size_t ip_datagram_size   = sizeof(struct ip) + sizeof(struct icmphdr) + MESSAGE_SIZE;

void pong() {
    int sockfd = 0; /* Deskryptor gniazda. */
    struct sockaddr reply_addr;
    struct icmphdr * icmp_hdr;
    struct ip * ip_hdr;
    char datagram[ip_datagram_size];
    char * message;

    sleep(1);

    ip_hdr   = (struct ip *) datagram;
    message  = (datagram + sizeof(struct ip) + sizeof(struct icmphdr));
    icmp_hdr = (struct icmphdr *) (datagram + sizeof(struct ip));

    int i;
    for (i = 0; i < 4; i++) {
        memset(datagram, 0, sizeof(datagram));
        socklen_t reply_addr_len = sizeof(struct sockaddr);

        sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

        recvfrom(sockfd, &datagram, sizeof(datagram), 0, &reply_addr, &reply_addr_len);

        fprintf(stderr, "Odebranie wiadomosci: \n");
        fprintf(stderr, "Wyslal: %s\n", inet_ntoa(ip_hdr->ip_src));
        fprintf(stderr, "TTL: %d\n", ip_hdr->ip_ttl);
        fprintf(stderr, "Dlugosc: %d\n", ip_hdr->ip_hl);
        fprintf(stderr, "Odebral: %s\n", inet_ntoa(ip_hdr->ip_dst));

        fprintf(stderr, "Typ: %d\n", icmp_hdr->type);
        fprintf(stderr, "Kod: %d\n", icmp_hdr->code);
        fprintf(stderr, "Identyfikator: %d\n", ntohs(icmp_hdr->un.echo.id));
        fprintf(stderr, "Numer sekwencyjny: %d\n", ntohs(icmp_hdr->un.echo.sequence));
        fprintf(stderr, "Wiadomosc: %s\n", message);
        fprintf(stderr, "****************\n");
    }
    close(sockfd);
} /* pong */

void ping(const char * dst_addr, int childpid) {
    struct icmphdr * icmp_header; // Naglowek ICMP:
    struct addrinfo hints;        // Struktura zawierajaca wskazowki dla funkcji getaddrinfo():

    /*
     * Wskaznik na liste zwracana przez getaddrinfo() oraz wskaznik uzywany do
     * poruszania sie po elementach listy:
     */
    struct addrinfo * rp, * result;

    int sockfd = 0;   //  Deskryptor gniazda.
    int ttl    = 100; //  Time To Live dla pakietu

    char datagram[icmp_datagram_size]; //  bufor na naglowek icmp i wiadomosc
    char * message;                    //  wskaznik na czesc wiadomosci w datagramie
    int retval;                        //  Wartosc zwracana przez funkcje.
    int i;

    icmp_header = (struct icmphdr *) datagram;
    message     = (datagram + sizeof(struct icmphdr));

    /* Wskazowki dla getaddrinfo(): */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_INET;      /* Domena komunikacyjna (IPv4). */
    hints.ai_socktype = SOCK_RAW;     /* Typ gniazda. */
    hints.ai_protocol = IPPROTO_ICMP; /* Protokol. */

    retval = getaddrinfo(dst_addr, NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        kill(childpid, SIGKILL);
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

        retval = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        if (retval == -1) {
            perror("setsockopt()");
            kill(childpid, SIGKILL);
            exit(EXIT_FAILURE);
        } else {
            /* Jezeli gniazdo zostalo poprawnie utworzone i
             * ttl: */
            break;
        }
    }

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Client failure: could not create socket.\n");
        kill(childpid, SIGKILL);
        exit(EXIT_FAILURE);
    }

    /*Wypełnianie wiadomosci*/
    strcpy(message, "Przykladowa wiadomosc");

    /* Wyslanie komunikatu ICMP Echo: */
    for (i = 1; i <= 4; i++) {
        sleep(1);
        /* Wypelnienie pol naglowka ICMP Echo: */
        memset(icmp_header, 0, sizeof(struct icmphdr));
        icmp_header->type             = ICMP_ECHO;
        icmp_header->code             = 0;
        icmp_header->un.echo.id       = htons((uint16_t) getpid());
        icmp_header->un.echo.sequence = htons((uint16_t) i);
        icmp_header->checksum         = internet_checksum((unsigned short *) datagram, (int) icmp_datagram_size);
        retval = (int) sendto(sockfd, datagram, sizeof(datagram), 0, rp->ai_addr, rp->ai_addrlen);
        if (retval == -1) {
            perror("sentdo()");
        }
        fprintf(stderr, "Wyslanie wiadomosci id %d\n", i);
    }
    freeaddrinfo(result);
    close(sockfd);
} /* ping */

int main(int argc, char ** argv) {
    int childpid;
    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <HOSTNAME OR IP ADDRESS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((childpid = fork()) == 0) {
        pong();
    } else {
        ping(argv[1], childpid);
    }

    wait(NULL);
    exit(EXIT_SUCCESS);
}
