#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <zconf.h>
#include <time.h>
#include <memory.h>
#include <arpa/inet.h>

#define TIME 0
#define DATE 1

#define true 1
#define false 0

#define MESSAGE_SIZE 100


typedef unsigned short boolean;

typedef struct {
    unsigned short type;
    char           message[MESSAGE_SIZE];
}                      message_t;

void test(int rval, const char *message, boolean kill);

void socket_w(int *fd);

void connect_w(const int *fd, struct sockaddr_in *server_addr);

void print_conn_info(const int *fd);

void handle_connection(const int *fd, struct sockaddr_in *server_addr);

int main(int argc, char *argv[]) {
    int                port;
    int                buff[100];
    char               *server_ip;
    int                fd;
    int                retval;
    struct sockaddr_in server_addr;
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

    connect_w(&fd, &server_addr);
    printf("Nawiązano połączenie z serwerem. Oczekiwanie na wiadomość\n");
    print_conn_info(&fd);

    handle_connection(&fd, &server_addr);
}

void handle_connection(const int *fd, struct sockaddr_in *server_addr) {
    message_t rcv_msg;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    int       retval;
    while (1) {
        memset(&rcv_msg, 0, sizeof(message_t));
        retval = sctp_recvmsg(*fd, &rcv_msg, sizeof(message_t), (struct sockaddr *) server_addr, &server_addr_len, 0,
                              0);
        test(retval, "sctp_recvmsg()", true);
        if (retval == 0) {
            printf("Serwer zakończył połączenie\n");
            break;
        }

        if (rcv_msg.type == DATE) {
            printf("Serwer wysłał datę:    %s\n", rcv_msg.message);
        } else if (rcv_msg.type == TIME) {
            printf("Serwer wysłał godzinę: %s\n", rcv_msg.message);
        } else {
            printf("Nieznany typ wiadomości\n");
        }
    }
}

void print_conn_info(const int *fd) {
    struct sctp_status status;
    socklen_t          status_len = sizeof(struct sctp_status);
    int                retval;

    retval = getsockopt(*fd, IPPROTO_SCTP, SCTP_STATUS, &status, &status_len);
    test(retval, "getsockopt()", true);

    printf("*****  Dane połączenia   *****\n");
    printf("Id:                        %d\n", status.sstat_assoc_id);
    printf("Stan:                      %d\n", status.sstat_state);
    printf("Strumienie wychodzące:     %d\n", status.sstat_outstrms);
    printf("Strumienie przychodzące:   %d\n", status.sstat_instrms);
    printf("*************************************\n");
}

void connect_w(const int *fd, struct sockaddr_in *server_addr) {
    int retval = connect(*fd, (const struct sockaddr *) server_addr, sizeof(*server_addr));
    test(retval, "connect()", true);
}

void socket_w(int *fd) {
    *fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
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
