/*
 * Data:                2009-03-01
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc server1.c -o server1
 * Uruchamianie:        $ ./server1 <numer portu>
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

    int                     listenfd, connfd;
    int                     retval, bytes;
    struct sockaddr_in      servaddr;
    char                    buffer[BUFF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT NUMBER>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family             =       AF_INET;
    servaddr.sin_port               =       htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr        =       htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 5) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    connfd = accept(listenfd, NULL, 0);
    if (connfd == -1) {
        perror("accept()");
        exit(EXIT_FAILURE);
    }

    for (;;) {

        bytes = recv(connfd, (void*)buffer, BUFF_SIZE, 0);
        if (bytes == -1) {
            perror("recv() failed");
            exit(EXIT_FAILURE);
        } else if (bytes == 0) {
            close(connfd);
            close(listenfd);
            exit(EXIT_SUCCESS);
        }

        retval = write(STDOUT_FILENO, buffer, bytes);

        if (send(connfd, buffer, bytes, 0) == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
        }
    }

}
