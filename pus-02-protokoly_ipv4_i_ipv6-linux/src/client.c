#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <zconf.h>
#include <netdb.h>

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

int getSocketFd(int domain) {
    int sfd = socket(domain, SOCK_STREAM, IPPROTO_TCP);
    validate(sfd, "socket()", true);

    if (domain == AF_INET6) {
        fprintf(stderr, "Socket type: ipv6\n");
    } else if (domain == AF_INET) {
        fprintf(stderr, "Socket type: ipv4\n");
    } else {
        fprintf(stderr, "Socket type: unknown\n");
    }

    return sfd;
}


int main(int argc, char *argv[]) {
    char               srvr_addr_name[NI_MAXHOST];
    in_port_t          port;
    int                client_fd;
    int                rval;
    char               message[256];
    struct addrinfo    hints;
    struct addrinfo    *result;

    if (argc != 3) {
        fprintf(stderr, "<program> <server address or name> <port>\n");
        exit(1);
    }

    memset(srvr_addr_name, 0, sizeof(srvr_addr_name));
    strcpy(srvr_addr_name, argv[1]);
    port = (in_port_t) strtol(argv[2], 0, 10);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if ((rval = getaddrinfo(srvr_addr_name, NULL, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rval));
        exit(EXIT_FAILURE);
    }

    client_fd = getSocketFd(result->ai_family);
    if (result->ai_family == AF_INET) {
        struct sockaddr_in * tmp;
        tmp = (struct sockaddr_in *) result->ai_addr;
        tmp->sin_port = htons(port);
    } else {
        struct sockaddr_in6 * tmp;
        tmp = (struct sockaddr_in6 *) result->ai_addr;
        tmp->sin6_port = htons(port);
    }

    fprintf(stderr, "Client data: \n");
    fprintf(stderr, "Address:   %s\n", srvr_addr_name);
    fprintf(stderr, "Port:      %d\n", port);
    fprintf(stderr, "************************************\n");

    rval = connect(client_fd, result->ai_addr, result->ai_addrlen);
    validate(rval, "connect()", true);

    memset(message, 0, sizeof(message));
    recv(client_fd, message, 255, 0);
    message[255] = '\0';

    fprintf(stderr, "Nawiazano polaczenie. Oto wiadomosc serwera: %s\n", message);

    close(client_fd);

    freeaddrinfo(result);
}