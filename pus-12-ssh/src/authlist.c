/*
 * Data:                2009-06-25
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc authlist.c -lssh2 libcommon.o -o authlist
 * Uruchamianie:        $ ./authlist [OPTIONS] <username>@<HOST>
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

#define BUF_SIZE 1024

/*
 * Funkcja odpowiedzialna za uwierzytelnianie uzytkownika.
 * Zdefiniowana na koncu pliku.
 */
int authenticate_user(LIBSSH2_SESSION *session, struct connection_data *cd);

int main(int argc, char **argv) {

    int                     err;
    int                     sockfd; /* Deskryptor gniazda */

    struct connection_data  *cd;   /* Argumenty wywolania programu */
    LIBSSH2_SESSION         *session; /* Obiekt sesji SSH */

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
 * "none".
 */
int authenticate_user(LIBSSH2_SESSION *session, struct connection_data *cd) {

    char    *auth_list;

    /* Pobranie listy metod udostepnianych przez serwer SSH. */
    auth_list = libssh2_userauth_list(session, cd->username, strlen(cd->username));
    if (auth_list == NULL) {
        /*
         *  Jezeli uwierzytelnianie metoda "none" zakonczylo sie
         * powodzeniem:
         */
        if (libssh2_userauth_authenticated(session)) {
            fprintf(stdout, "USERAUTH_NONE succeeded!\n");
            return 0;
        } else { /* W przypadku bledu: */
            print_ssh_error(session, "libssh2_userauth_list()");
            return -1;
        }
    }

    /* Wypisanie metod udostepnianych przez serwer SSH: */
    fprintf(stdout, "Authentication list: %s\n", auth_list);

    return 0;
}
