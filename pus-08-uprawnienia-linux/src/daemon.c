/*
 * Data:                2009-04-25
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc daemon.c -o daemon
 * Uruchamianie:        $ ./daemon <numer portu> <uid>
 */

#define _GNU_SOURCE     /* getresgid() */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <fcntl.h>
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>

int daemonize();


int main(int argc, char** argv) {

    /* Deskryptory dla gniazda nasluchujacego i polaczonego: */
    int             listenfd, connfd;

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    unsigned int    port_number;

    char*           str     =       "Daemon - laboratorium PUS\n";

    int             socket_option;


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number     =       atoi(argv[1]);

    /* Utworzenie gniazda dla protokolu TCP: */
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /*
     * Pozwolenie na ponowne wykorzystanie numeru portu przez gniazdo nasluchujace.
     * Przydatne w tym przykladzie, poniewaz serwer wykonuje active close i
     * polaczenie TCP po wywolaniu funkcji close() jest w stanie TIME_WAIT.
     */
    socket_option = 1;
    if (setsockopt(listenfd,
                   SOL_SOCKET, SO_REUSEADDR,
                   &socket_option, sizeof(int)
                  ) == -1) {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr     =       htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port            =       htons(port_number);
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);

    fprintf(stdout, "Binding to port %u...\n", port_number);
    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(listenfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    /* Przeksztalcenie gniazda w gniazdo nasluchujace: */
    if (listen(listenfd, 2) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is listening for incoming connection...\n");

    /* Demonizacja: */
    if (daemonize() == -1) {
        fprintf(stderr, "daemon() failed!\n");
        exit(EXIT_FAILURE);
    }

    /* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
     * i zwraca deskryptor dla gniazda polaczonego: */
    client_addr_len = sizeof(client_addr);
    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (connfd == -1) {
        exit(EXIT_FAILURE);
    }

    if (send(connfd, str, strlen(str), 0) == -1) {
        exit(EXIT_FAILURE);
    }

    close(connfd);
    close(listenfd);
    exit(EXIT_SUCCESS);
}

/* Demon to proces uruchomiony w tle i uniezalezniony od
 * termianalu kontrolujacego. */
int daemonize() {

    int fd;

    /* Utworzenie nowego procesu (demona): */
    switch (fork()) {
    case -1:
        return -1;
    case 0:
        break;
    default:
        /* Rodzic konczy dzialanie. Gwarantuje to, ze proces
         * potomny nie jest liderem grupy (proces potomny
         * dziedziczy 'process group ID' rodzica, ale otrzymuje
         * nowy PID). */
        exit(EXIT_SUCCESS);
    }

    /*
     * Proces potomny staje sie liderem grupy oraz liderem sesji.
     * Proces potomny po utworzeniu nowej sesji nie jest zwiazany z termin.
     * kontrolujacym.
     */
    if (setsid() == -1) {
        return -1;
    }

    /*
     * Zmiana katalogu roboczego na /. Katalog roboczy jest dziedziczony po
     * procesie macierzystym. Katalog roboczy moze znajdowac sie na
     * zamontowanym systemie plikow i uniemozliwiac jego odmontowanie.
     * Z tego wzgledu zmiana na / moze byc pomocna.
     */
    if (chdir("/") == -1) {
        return -1;
    }

    /* "Przekierowanie" deskryptorow 0, 1 ,2 na /dev/null.
     * Demon nie bedzie czytal ze standardowego wejscia i pisal na
     * standardowe wyjscie.
     */
    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        return -1;
    }

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > 2) {
        close(fd);
    }

    return 0;
}
/*
 * GLIBC dostarcza funkcje daemon(), ktora realizuje ta sama funkcjonalnosc.
 * Funkcja daemon() pozwala opcjonalnie zachowac bierzacy katalog roboczy oraz
 * deskryptory plikow: 0, 1 i 2.
 */
