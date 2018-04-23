#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <zconf.h>
#include <time.h>
#include <memory.h>

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

void bind_w(const int *fd, int port);

void listen_w(const int *fd);

int accept_w(const int *fd, struct sockaddr_in *client_addr);

void sockopt_config(const int *fd);

void send_messages(const int *fd, struct sockaddr_in *client_addr);

int main(int argc, char *argv[]) {
    int port;
    int fd;
    if (argc != 2) {
        fprintf(stderr, "<program> <port>\n");
        exit(EXIT_FAILURE);
    }

    port = (int) strtol(argv[1], 0, 10);

    socket_w(&fd);
    bind_w(&fd, port);
    listen_w(&fd);
    sockopt_config(&fd);

    printf("Gniazdo skonfigurowane\n");
    while (1) {
        struct sockaddr_in client_addr;
        int                conn_fd = accept_w(&fd, &client_addr);
        send_messages(&conn_fd, &client_addr);
        close(conn_fd);
    }
}

void send_messages(const int *fd, struct sockaddr_in *client_addr) {
    time_t current_time;
    struct tm *current_time_tm;
    char      time_buff[MESSAGE_SIZE];
    char      date_buff[MESSAGE_SIZE];
    int       retval;
    message_t message_date, message_time;

    memset(time_buff, 0, MESSAGE_SIZE);
    memset(date_buff, 0, MESSAGE_SIZE);

    time(&current_time);
    current_time_tm = localtime(&current_time);

    strftime(time_buff, MESSAGE_SIZE, "%R", current_time_tm);
    strftime(date_buff, MESSAGE_SIZE, "%F", current_time_tm);

    printf("Aktualna data:    %s\n", date_buff);
    printf("Aktualna godzina: %s\n", time_buff);

    message_time.type = TIME;
    memset(message_date.message, 0, MESSAGE_SIZE);
    strcpy(message_time.message, time_buff);


    message_date.type = DATE;
    memset(message_date.message, 0, MESSAGE_SIZE);
    strcpy(message_date.message, date_buff);

    retval = sctp_sendmsg(*fd, &message_time, sizeof(message_t), (struct sockaddr *) client_addr, sizeof(*client_addr),
                          (uint32_t) getpid(), 0, 0, 0, 0);
    test(retval, "send time error", false);


    retval = sctp_sendmsg(*fd, &message_date, sizeof(message_t), (struct sockaddr *) client_addr, sizeof(*client_addr),
                          (uint32_t) getpid(), 0, 0, 0, 0);
    test(retval, "send date error", false);
}


int accept_w(const int *fd, struct sockaddr_in *client_addr) {
    int conn_fd;
    printf("Oczekiwnaie na klienta\n");
    socklen_t socklen = sizeof(struct sockaddr_in);
    conn_fd = accept(*fd, (struct sockaddr *) client_addr, &socklen);
    test(conn_fd, "accept()", true);
    printf("Nazwiązano połączenie z nowym klientem.\nWysyłanie danych\n\n");
    return conn_fd;
}

void sockopt_config(const int *fd) {
    struct sctp_initmsg initmsg;

    initmsg.sinit_num_ostreams  = 3;
    initmsg.sinit_max_instreams = 4;
    initmsg.sinit_max_attempts  = 5;

    setsockopt(*fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
}


void listen_w(const int *fd) {
    int rval;
    rval = listen(*fd, 5);

    test(rval, "listen()", true);
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
