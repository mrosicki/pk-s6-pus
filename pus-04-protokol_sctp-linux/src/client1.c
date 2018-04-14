/*
 * Data:                2009-03-01
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc client1.c -o client1
 * Uruchamianie:        $ ./client1 <adres IP> <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFF_SIZE 256

int main(int argc, char** argv) {

    int                     sockfd;
    int                     retval, bytes;
    char                    *retptr;
    struct addrinfo hints, *result;
    char                    buff[BUFF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IP ADDRESS> <PORT NUMBER>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype       = SOCK_STREAM;
    hints.ai_flags          = 0;
    hints.ai_protocol       = 0;

    retval = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    if (result == NULL) {
        fprintf(stderr, "Could not connect!\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(result->ai_family, result->ai_socktype, IPPROTO_SCTP);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    for (;;) {

        retptr = fgets(buff, BUFF_SIZE, stdin);
        if ((retptr == NULL) || (strcmp(buff, "\n") == 0)) {
            break;
        }

        bytes = send(sockfd, buff, strlen(buff), 0);
        if (bytes == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
        }

        bytes = recv(sockfd, (void*)buff, BUFF_SIZE, 0);
        if (bytes == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "Echo: ");
        fflush(stdout);
        retval = write(STDOUT_FILENO, buff, bytes);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
