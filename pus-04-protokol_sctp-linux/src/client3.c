#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <zconf.h>
#include <time.h>
#include <memory.h>
#include <arpa/inet.h>

#define BUFF_SIZE 250

#define MAX_STREAMS 10

#define true 1
#define false 0

typedef unsigned short boolean;

void socket_w(int *fd);

void sockopt_config(const int *fd);

void test(int rval, const char *message, boolean kill);

int main(int argc, char *argv[]) {
    int                port;
    int                buff[100];
    char               *server_ip;
    int                fd;
    int                retval;
    struct sockaddr_in server_addr;
    int                next_stream = 0;
    if (argc != 3) {
        fprintf(stderr, "<program> <ip> <port>\n");
        exit(EXIT_FAILURE);
    }

    server_ip = argv[1];
    port      = (int) strtol(argv[2], 0, 10);

    socket_w(&fd);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons((uint16_t) port);
    retval = inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    test(retval, "inet_pton()", true);

    sockopt_config(&fd);

    printf("Klient skonfigurowany. Napisz wiadomość\n");
    while (1) {
        char                   message[BUFF_SIZE];
        struct sockaddr_in     recv_addr;
        struct sctp_sndrcvinfo info;
        socklen_t              recv_addr_len = sizeof(recv_addr);

        memset(&message, 0, BUFF_SIZE);
        printf("$: ");
        fgets(message, BUFF_SIZE, stdin);
        if (strcmp(message, "\n") == 0) {
            printf("Program kończy pracę\n");
            break;
        }
        message[strlen(message) - 1] = 0; // ucinanie znaku nowej linii

        printf("Wiadomość do wysłania: %s\n", message);

        retval = sctp_sendmsg(fd, message, BUFF_SIZE, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in),
                              0, 0, (uint16_t) next_stream, 0, 0);
        test(retval, "sctp_sendmsg()", true);

        memset(message, 0, BUFF_SIZE);
        memset(&recv_addr, 0, sizeof(recv_addr));
        memset(&info, 0, sizeof(struct sctp_sndrcvinfo));
        retval = sctp_recvmsg(fd, message, BUFF_SIZE, (struct sockaddr *) &recv_addr, &recv_addr_len, &info, 0);
        test(retval, "sctp_recvmsg()", true);

        next_stream = info.sinfo_stream;

        printf("Dane odebrane od serwera:\n");
        printf("Wiadomosc:    %s\n", message);
        printf("Id asocjacji: %d\n", info.sinfo_assoc_id);
        printf("Strumień:     %d\n", info.sinfo_stream);
        printf("SSN:          %d\n", info.sinfo_ssn);
        printf("*****************************************\n");
    }

    close(fd);
}

void socket_w(int *fd) {
    *fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    test(*fd, "socket()", true);
}


void test(int rval, const char *message, boolean kill) {
    if (rval == -1) {
        perror(message);

        if (kill == true) {
            exit(EXIT_FAILURE);
        }
    }
}

void sockopt_config(const int *fd) {
    struct sctp_initmsg         initmsg;
    struct sctp_event_subscribe events;
    initmsg.sinit_num_ostreams  = MAX_STREAMS;
    initmsg.sinit_max_instreams = MAX_STREAMS;
    initmsg.sinit_max_attempts  = 5;

    setsockopt(*fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));

    events.sctp_data_io_event = 1;
    setsockopt(*fd, SOL_SCTP, SCTP_EVENTS, (const void *) &events, sizeof(events));
}