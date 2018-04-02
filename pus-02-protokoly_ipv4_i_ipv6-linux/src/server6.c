#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#define PORT 5432

#define true 1
#define false 0

typedef int boolean;

void validate(int val, const char *errmessage, boolean kill) {
    if (val == -1) {
        fprintf(stderr, "%s\n", errmessage);
        if (kill == true) {
            exit(EXIT_FAILURE);
        }
    }
}

int getSocketFd() {
    int sfd = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
    validate(sfd, "socket()", true);
    return sfd;
}

int getBindedFd() {
    int                 sfd, rval;
    struct sockaddr_in6 srvr_addr;
    socklen_t           srvr_addr_len;

    sfd = getSocketFd();
    memset(&srvr_addr, 0, sizeof(struct sockaddr));
    srvr_addr.sin6_family = AF_INET6;
    srvr_addr.sin6_port   = htons(PORT);
    srvr_addr.sin6_addr   = in6addr_any;
    srvr_addr_len = sizeof(srvr_addr);

    rval = bind(sfd, (const struct sockaddr *) &srvr_addr, srvr_addr_len);
    validate(rval, "bind()", true);

    return sfd;
}

char *getIpv6PAddr(int sfd, char *buff, int *port) {
    struct sockaddr_in6 addripv6;
    socklen_t          addripv6_len = sizeof(addripv6);
    int                rval;

    memset(&addripv6, 0, addripv6_len);
    rval = getsockname(sfd, (struct sockaddr *) &addripv6, &addripv6_len);
    validate(rval, "error ipv6 getsockname()", false);

    inet_ntop(AF_INET6, &(addripv6.sin6_addr), buff, INET6_ADDRSTRLEN);
    *(port) = ntohs(addripv6.sin6_port);

    return buff;
}

void printServerData(int sfd) {
    char paddripv6[INET6_ADDRSTRLEN];
    int  port;

    getIpv6PAddr(sfd, paddripv6, &port);

    fprintf(stderr, "Server connection data: \n");
    fprintf(stderr, "Ipv6 addr: %s\n", paddripv6);
    fprintf(stderr, "Port:      %d\n", port);
    fprintf(stderr, "***************************************\n");
}

void printClientData(struct sockaddr_in6 *sockaddripv6) {
    char paddripv6[INET6_ADDRSTRLEN];
    int  port;

    inet_ntop(AF_INET6, &(sockaddripv6->sin6_addr), paddripv6, INET6_ADDRSTRLEN);
    port = ntohs(sockaddripv6->sin6_port);

    fprintf(stderr, "Polaczenie z nowym klientem\n");
    fprintf(stderr, "Adres ipv6: %s\n", paddripv6);
    fprintf(stderr, "Port:       %d\n", port);

    if (IN6_IS_ADDR_V4MAPPED(&(sockaddripv6->sin6_addr))) {
        fprintf(stderr, "!!! Adres jest mapowany z ipv4 !!!\n");
    }
}

int getListenFd() {
    int sfd, rval;

    sfd  = getBindedFd();
    rval = listen(sfd, 1);
    validate(rval, "listen()", true);

    return sfd;
}

void sendReturnMessage(int connfd) {
    char    message[] = "Laboratorium PUS";
    ssize_t rval;

    rval = send(connfd, message, sizeof(message), 0);
    validate((int) rval, "send()", true);

    fprintf(stderr, "Wyslano wiadomosc: %s\n", message);
}

int main() {
    int listen_fd;

    struct sockaddr_in6 client_addr;
    socklen_t          client_addr_len;

    listen_fd = getListenFd();
    printServerData(listen_fd);

    while (true) {
        int connfd;
        memset(&client_addr, 0, sizeof(struct sockaddr_in6));
        client_addr_len = sizeof(struct sockaddr_in6);

        connfd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        validate(connfd, "accept()", true);

        printClientData(&client_addr);
        sendReturnMessage(connfd);
        fprintf(stderr, "***************************************\n");
    }
}