/*
 * Data:                2009-02-10
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc client2.c -o client2
 * Uruchamianie:        $ ./client2 <adres IP> <numer portu> <wiadomosc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>

int main(int argc, char** argv) {

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t       addr_len;               /* Rozmiar struktury w bajtach. */
    char            buff[256];              /* Bufor dla funkcji recvfrom(). */


    if (argc != 4) {
        fprintf(
            stderr,
            "Invocation: %s <IPv4 ADDRESS> <PORT> <MESSAGE>\n", argv[0]
        );
        exit(EXIT_FAILURE);
    }

    /* Przyciecie wiadomosci, jezeli jest za dluga: */
    if (strlen(argv[3]) > 255) {
        fprintf(stdout, "Truncating message.\n");
        argv[3][255] = '\0';
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    remote_addr.sin_family = AF_INET;

    /* Konwersja adresu IP z postaci kropkowo-dziesietnej: */
    retval = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    remote_addr.sin_port = htons(atoi(argv[2])); /* Numer portu. */
    addr_len = sizeof(remote_addr); /* Rozmiar struktury adresowej w bajtach. */

    sleep(1);

    fprintf(
        stdout,
        "Sending message to %s.\nWaiting for server response...\n", argv[1]
    );

    sleep(2);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
                 sockfd,
                 argv[3], strlen(argv[3]),
                 0,
                 (struct sockaddr*)&remote_addr, addr_len
             );
    if (retval == -1) {
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    /* Oczekiwanie na odpowiedz.
     * Nie interesuje nas adres, z ktorego wyslano odpowiedz: */
    retval = recvfrom(sockfd, buff, sizeof(buff), 0, NULL, NULL);
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }

    buff[retval] = '\0';

    fprintf(stdout, "Server response: '%s'\n", buff);

    close(sockfd);

    exit(EXIT_SUCCESS);
}
