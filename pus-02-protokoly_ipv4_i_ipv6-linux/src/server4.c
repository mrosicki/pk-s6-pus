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
    int sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    validate(sfd, "socket()", true);
    return sfd;
}

int getBindedFd() {
    int                sfd, rval;
    struct sockaddr_in srvr_addr;
    socklen_t          srvr_addr_len;

    sfd = getSocketFd();
    memset(&srvr_addr, 0, sizeof(struct sockaddr));
    srvr_addr.sin_family      = AF_INET;
    srvr_addr.sin_port        = htons(PORT);
    srvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvr_addr_len = sizeof(srvr_addr);

    rval = bind(sfd, (const struct sockaddr *) &srvr_addr, srvr_addr_len);
    validate(rval, "bind()", true);

    return sfd;
}

char *getIpv4PAddr(int sfd, char *buff, int *port) {
    struct sockaddr_in addripv4;
    socklen_t          addripv4_len = sizeof(addripv4);
    int                rval;

    memset(&addripv4, 0, addripv4_len);
    rval = getsockname(sfd, (struct sockaddr *) &addripv4, &addripv4_len);
    validate(rval, "error ipv4 getsockname()", false);

    inet_ntop(AF_INET, &(addripv4.sin_addr), buff, INET_ADDRSTRLEN);
    *(port) = ntohs(addripv4.sin_port);

    return buff;
}

void printServerData(int sfd) {
    char paddripv4[INET_ADDRSTRLEN];
    int  port;

    getIpv4PAddr(sfd, paddripv4, &port);

    fprintf(stderr, "Server connection data: \n");
    fprintf(stderr, "Ipv4 addr: %s\n", paddripv4);
    fprintf(stderr, "Port:      %d\n", port);
    fprintf(stderr, "***************************************\n");
}

void printClientData(struct sockaddr_in *sockaddripv4) {
    char paddripv4[INET_ADDRSTRLEN];
    int  port;

    inet_ntop(AF_INET, &(sockaddripv4->sin_addr), paddripv4, INET_ADDRSTRLEN);
    port = ntohs(sockaddripv4->sin_port);

    fprintf(stderr, "Polaczenie z nowym klientem\n");
    fprintf(stderr, "Adres ipv4: %s\n", paddripv4);
    fprintf(stderr, "Port:       %d\n", port);
}

int getListenFd() {
    int sfd, rval;

    sfd  = getBindedFd();
    rval = listen(sfd, 1);
    validate(rval, "listen()", true);

    printServerData(sfd);

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

    struct sockaddr_in client_addr;
    socklen_t          client_addr_len;

    listen_fd = getListenFd();

    while (true) {
        int connfd;
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        client_addr_len = sizeof(struct sockaddr_in);

        connfd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        validate(connfd, "accept()", true);

        printClientData(&client_addr);
        sendReturnMessage(connfd);
        fprintf(stderr, "***************************************\n");
    }
}