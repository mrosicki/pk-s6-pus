#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <zconf.h>

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


int main(int argc, char *argv[]) {
    char                ipv6Addr[INET6_ADDRSTRLEN];
    in_port_t           port;
    unsigned int        if_index;
    char                if_name[64];
    int                 client_fd;
    struct sockaddr_in6 srvr_addr;
    socklen_t           srvr_addr_len;
    int                 rval;
    char                message[256];


    if (argc != 4) {
        fprintf(stderr, "<program> <server address> <port> <interface>\n");
        exit(1);
    }

    strcpy(ipv6Addr, argv[1]);
    port = (in_port_t) strtol(argv[2], 0, 10);
    strcpy(if_name, argv[3]);


    if_index = if_nametoindex(if_name);
    if (if_index == 0) {
        perror("if_nametoindex()\n");
        exit(1);
    }

    client_fd = getSocketFd();


    memset(&srvr_addr, 0, sizeof(srvr_addr));
    inet_pton(AF_INET6, ipv6Addr, &(srvr_addr.sin6_addr));
    srvr_addr.sin6_family   = AF_INET6;
    srvr_addr.sin6_port     = htons(port);
    srvr_addr.sin6_scope_id = if_index;
    srvr_addr_len = sizeof(srvr_addr);

    fprintf(stderr, "Client data: \n");
    fprintf(stderr, "Address:   %s\n", ipv6Addr);
    fprintf(stderr, "Port:      %d\n", port);
    fprintf(stderr, "Interface: %d\n", if_index);
    fprintf(stderr, "************************************\n");

    rval = connect(client_fd, (const struct sockaddr *) &srvr_addr, srvr_addr_len);
    validate(rval, "connect()", true);

    memset(message, 0, sizeof(message));
    recv(client_fd, message, 255, 0);
    message[255] = '\0';

    fprintf(stderr, "Nawiazano polaczenie. Oto wiadomosc serwera: %s\n", message);

    close(client_fd);
}