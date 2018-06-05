/*
 * Data:                2009-06-25
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc exec.c -lssh2 libcommon.o -o exec
 * Uruchamianie:        $ ./exec [OPTIONS] <username>@<HOST>
 *                      OPTIONS:
 *                        -p <port number>
 *                        Port to connect to on the remote host; default: 22
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <libssh2.h>
#include "libcommon.h"

#define PASS_LEN 128
#define BUF_SIZE 4096

/* Funkcja odpowiedzialna za uwierzytelnianie uzytkownika. */
int authenticate_user(LIBSSH2_SESSION *session, struct connection_data *cd);

int main(int argc, char **argv) {

    int                     err;
    int                     sockfd; /* Deskryptor gniazda */
    char                    buf[BUF_SIZE]; /* Bufor dla odbieranych danych */
    ssize_t                 bytes, total; /* Liczba otrzymanych bajtow */

    struct connection_data  *cd; /* Argumenty wywolania programu */
    LIBSSH2_SESSION         *session; /* Obiekt sesji SSH */
    LIBSSH2_CHANNEL         *channel; /* Kanal komunikacyjny */

    /*
     * Parsowanie argumentow wywolania programu.
     * Wymagane argumenty to adres IPv4 oraz nazwa uzytkownika.
     */
    cd = parse_connection_data(argc, argv, CD_ADDRESS | CD_USERNAME);
    if (cd == NULL) {
        exit(EXIT_FAILURE);
    }

    /*
     * Nawiazanie polaczenia TCP. Zwracany jest deskryptor dla gniazda polaczonego.
     * Dane potrzebne do ustanowienia polaczenia sa przekazywane za pomoca
     * wskaznika na strukture connection_data.
     */
    sockfd = establish_tcp_connection(cd);
    if (sockfd == -1) {
        exit(EXIT_FAILURE);
    }

    /*
     * Inicjalizacja struktury reprezentujacej sesje protokolu Transport Layer
     * Protocol.
     */
    session = libssh2_session_init();
    if (session == NULL) {
        fprintf(stderr, "libssh2_session_init() failed!\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Rozpoczecie negocjacji parametrow polaczenia SSH, wymiana tajnego klucza i
     * uwierzytelnianie serwera SSH.
     */
    err = libssh2_session_startup(session, sockfd);
    if (err < 0) {
        print_ssh_error(session, "libssh2_session_startup()");
        exit(EXIT_FAILURE);
    }

    /*
     * Klucz publiczny serwera jest weryfikowany na podstawie cyfrowego odcisku
     * palca. Uzytkownik jest pytany, czy klucz o danym odcisku moze zostac
     * zaakceptowany.
     */
    err = authenticate_server(session);
    if (err < 0) {
        exit(EXIT_FAILURE);
    }

    /* Uwierzytelnianie uzytkownika. Funkcja zdefiniowana na koncu tego pliku. */
    err = authenticate_user(session, cd);
    if (err < 0) {
        exit(EXIT_FAILURE);
    }

    /* Utworzenie kanalu komunikacyjnego typu "session". */
    channel = libssh2_channel_open_session(session);
    if (channel == NULL) {
        print_ssh_error(session, "linssh2_channel_open_session()");
        exit(EXIT_FAILURE);
    }

    /* Zdalne wywolanie polecenia systemowego/programu. */
    err = libssh2_channel_exec(channel, "date");
    if (err < 0) {
        print_ssh_error(session, "libssh2_channel_exec()");
        exit(EXIT_FAILURE);
    }

    /* Odebranie danych: */
    bytes = total = 0;
    do {
        bytes = libssh2_channel_read(channel, buf + total, BUF_SIZE - total);
        total += bytes;
    } while ((bytes > 0) && (total < BUF_SIZE));

    if (bytes < 0) {
        print_ssh_error(session, "libssh2_channel_read()");
        exit(EXIT_FAILURE);
    }

    /* Wypisanie odpowiedzi: */
    fprintf(stdout, "Received:\n");
    fwrite(buf, sizeof(char), total, stdout);
    fprintf(stdout, "\n");

    /* Wypisanie statusu zakonczenie zdalnego procesu. */
    fprintf(stdout, "Exit status: %d\n", libssh2_channel_get_exit_status(channel));

    /* Zamkniecie kanalu komunikacyjnego. */
    err = libssh2_channel_close(channel);
    if (err < 0) {
        print_ssh_error(session, "libssh2_channel_close()");
        exit(EXIT_FAILURE);
    }

    /* Zwolnienie zasobow zwiazanych z kanalem komunikacyjnym. */
    err = libssh2_channel_free(channel);
    if (err < 0) {
        print_ssh_error(session, "libssh2_channel_free()");
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie sesji protokolu Transport Layer Protocol (polaczenia SSH). */
    err = libssh2_session_disconnect(session,  "Graceful shutdown");
    if (err < 0) {
        print_ssh_error(session, "libssh2_session_disconnect()");
        exit(EXIT_FAILURE);
    }

    /* Zwolnienie zasobow zwiazanych z sesja SSH. */
    err = libssh2_session_free(session);
    if (err < 0) {
        print_ssh_error(session, "libssh2_session_free()");
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie gniazda TCP. */
    close(sockfd);

    /* Zwolnienie pamieci zaalokowanej dla argumentow wywolania programu. */
    free_connection_data(cd);

    exit(EXIT_SUCCESS);
}


/*
 * Funkcja odpowiedzialna za uwierzytelnianie uzytkownika za pomoca metody
 * "password".
 */
int authenticate_user(LIBSSH2_SESSION *session, struct connection_data *cd) {

    char    *auth_list;
    char    password[PASS_LEN];

    /* Pobranie listy metod udostepnianych przez serwer SSH. */
    auth_list = libssh2_userauth_list(session, cd->username, strlen(cd->username));
    if (auth_list == NULL) {
        /*
         *  Jezeli uwierzytelnianie metoda "none" zakonczylo sie
         * powodzeniem:
         */
        if (libssh2_userauth_authenticated(session)) {
            fprintf(stderr, "USERAUTH_NONE succeeded!\n");
            return -1;
        } else { /* W przypadku bledu: */
            print_ssh_error(session, "libssh2_userauth_list()");
            return -1;
        }
    }

    /* Sprawdzenie czy serwer obsluguje metode "password". */
    if (strstr(auth_list, "password") == NULL) {
        fprintf(stderr, "Password method not supported by server.\n");
        return -1;
    }

    for (;;) {

        /* Pobranie hasla uzytkownika. */
        if (get_password("Password: ", password, PASS_LEN)) {
            fprintf(stderr, "get_password() failed!\n");
            return -1;
        }

        /* Uwierzytelnianie za pomoca hasla. */
        if (libssh2_userauth_password(session, cd->username, password) == 0) {
            fprintf(stdout, "Authentication succeeded!\n");
            memset(password, 0, PASS_LEN);
            break;
        }  else {
            fprintf(stdout, "Authentication failed!\n");
        }
    }

    return 0;
}
