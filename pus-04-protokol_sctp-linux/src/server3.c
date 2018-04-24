#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/sctp.h>
#include <zconf.h>
#include <memory.h>

#define BUFF_SIZE 250

#define true 1
#define false 0

#define MAX_STREAMS 10

int next_stream;

typedef unsigned short boolean;

void test(int rval, const char *message, boolean kill);

void socket_w(int *fd);

void bind_w(const int *fd, int port);

void listen_w(const int *fd);

void sockopt_config(const int *fd);

int accept_w(const int *fd, struct sockaddr_in *client_addr);

int main(int argc, char *argv[]) {
    int fd;
    int port;
    int reply_type;

    if (argc != 3) {
        fprintf(stderr, "<program> <port> <0 lub 1>\n");
        exit(EXIT_FAILURE);
    }

    port       = (int) strtol(argv[1], 0, 10);
    reply_type = (int) strtol(argv[2], 0, 10);

    socket_w(&fd);
    bind_w(&fd, port);
    listen_w(&fd);

    sockopt_config(&fd);

    printf("Gniazdo skonfigurowane\n");
    while (1) {
        struct sockaddr_in     client_addr;
        socklen_t              client_addr_len = sizeof(struct sockaddr_in);
        char                   message[BUFF_SIZE];
        int                    retval;
        struct sctp_sndrcvinfo info;

        memset(message, 0, BUFF_SIZE);
        memset(&client_addr, 0, sizeof(client_addr));
        memset(&info, 0, sizeof(struct sctp_sndrcvinfo));
        retval = sctp_recvmsg(fd, message, BUFF_SIZE, (struct sockaddr *) &client_addr, &client_addr_len, &info, 0);
        test(retval, "sctp_recvmsg()", true);
        printf("Wiadomość klienta: %s\n", message);


        if (reply_type == 1) {
            next_stream = (info.sinfo_stream + 1) % MAX_STREAMS;
        }

        retval = sctp_sendmsg(fd, message, strlen(message), (struct sockaddr *) &client_addr, client_addr_len, 0, 0,
                              (uint16_t) next_stream, 0, 0);
        test(retval, "sctp_sendmsg()", true);

        printf("Odesłanie danych strumieniem %d\n", next_stream);
        printf("***************************************\n");
    }
}

void test(int rval, const char *message, boolean kill) {
    if (rval == -1) {
        perror(message);

        if (kill == true) {
            exit(EXIT_FAILURE);
        }
    }
}

void socket_w(int *fd) {
    *fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    test(*fd, "socket()", true);
}

void bind_w(const int *fd, int port) {
    struct sockaddr_in server_addr;
    int                rval;

    server_addr.sin_port        = htons((uint16_t) port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family      = AF_INET;
    rval = bind(*fd, (const struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));

    test(rval, "bind()", true);
}

void listen_w(const int *fd) {
    int rval;
    rval = listen(*fd, 5);
    test(rval, "listen()", true);
}

void sockopt_config(const int *fd) {
    struct sctp_initmsg         initmsg;
    struct sctp_event_subscribe events;

    memset(&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams  = MAX_STREAMS;
    initmsg.sinit_max_instreams = MAX_STREAMS;
    initmsg.sinit_max_attempts  = 5;
    setsockopt(*fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));

    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;
    setsockopt(*fd, SOL_SCTP, SCTP_EVENTS, (const void *) &events, sizeof(events));
}

int accept_w(const int *fd, struct sockaddr_in *client_addr) {
    int conn_fd;
    printf("Oczekiwnaie na klienta\n");
    socklen_t socklen = sizeof(struct sockaddr_in);
    conn_fd = accept(*fd, (struct sockaddr *) client_addr, &socklen);
    test(conn_fd, "accept()", true);
    printf("Nazwiązano połączenie z nowym klientem.\n\n");
    return conn_fd;
}



