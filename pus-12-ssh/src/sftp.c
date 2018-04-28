/*
 * Data:                2009-06-25
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc sftp.c -lssh2 libcommon.o -o sftp
 * Uruchamianie:        $ ./sftp [OPTIONS] <username>@<HOST>
 *                      OPTIONS:
 *                        -p <port number>
 *                        Port to connect to on the remote host; default: 22
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include "libcommon.h"

#define PASS_LEN 128
#define BUF_SIZE 1024

int authenticate_user(LIBSSH2_SESSION *session, struct connection_data *cd,
                      const char* pubkey, const char* privkey,
                      const char* passphrase);

int main(int argc, char **argv) {

    int                     err;
    int                     sockfd; /* Deskryptor gniazda */
    char                    dir[BUF_SIZE]; /* Bufor na sciezke do katalogu */
    char                    filename[BUF_SIZE]; /* Nazwa pliku */
    char                    mem[BUF_SIZE]; /* Bufor dla operacji I/O */
    int                     bytes; /* Liczba bajtow */

    struct connection_data  *cd; /* Argumenty wywolania programu */
    LIBSSH2_SESSION         *session; /* Obiekt sesji SSH */
    LIBSSH2_SFTP            *sftp; /* Sesja podsystemu "sftp" */
    LIBSSH2_SFTP_HANDLE     *handle; /* Uchwyt SFTP */

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

    /*
     * Uwierzytelnianie uzytkownika ("publickey", "password").
     * Funkcja zdefiniowana na koncu tego pliku.
     */
    err = authenticate_user(session, cd, "public.key", "private.key", "passphrase");
    if (err < 0) {
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja podsystemu "sftp". */
    sftp = libssh2_sftp_init(session);
    if (sftp == NULL) {
        fprintf(stderr, "libssh2_sftp_init() failed!\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Zdefiniowanie sciezki do katalogu domowego uzytkownika.
     * !!! Moze wymagac modyfikacji !!!
     */
    strcpy(dir, "/home/");
    strcat(dir, cd->username);

    /* Pobranie uchwytu SFTP do zdalnego katalogu. */
    handle = libssh2_sftp_opendir(sftp,  dir);
    if (handle == NULL) {
        print_sftp_error(session, sftp, "libssh2_sftp_opendir()");
        exit(EXIT_FAILURE);
    }

    /* Listowanie zawartosci katalogu: */
    do {
        bytes = libssh2_sftp_readdir_ex(handle, filename, BUF_SIZE,
                                        mem,  BUF_SIZE,  NULL);


        if (strlen(mem) > 0) {
            /* Jezeli jest dostepna szczegolowa wersja listingu. */
            fprintf(stdout, "%s\n", mem);
        } else {
            fprintf(stdout, "%s\n", filename);
        }


    } while (bytes > 0);

    if (bytes < 0) {
        print_sftp_error(session, sftp, "libssh2_sftp_readdir_ex()");
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie uchwytu SFTP. */
    err = libssh2_sftp_closedir(handle);
    if (err < 0) {
        print_sftp_error(session, sftp, "libssh2_sftp_closedir()");
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie polaczenia SFTP. */
    err = libssh2_sftp_shutdown(sftp);
    if (err < 0) {
        print_sftp_error(session, sftp, "libssh2_sftp_shutdown()");
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
 * Probuje uwierzytelniania za pomoca klucza publicznego, a jezeli nie jest
 * obslugiwane przez serwer lub zakonczy sie niepowodzeniem, to wykorzystuje
 * metode "password".
 */
int authenticate_user(LIBSSH2_SESSION *session, struct connection_data *cd,
                      const char* pubkey, const char* privkey,
                      const char* passphrase) {

    int     err;
    char    *auth_list;
    char    password[PASS_LEN];

    /* Pobranie listy metod udostepnianych przez serwer SSH. */
    auth_list = libssh2_userauth_list(session, cd->username, strlen(cd->username));
    if (auth_list == NULL) {
        if (libssh2_userauth_authenticated(session)) {
            fprintf(stderr, "USERAUTH_NONE succeeded!\n");
            return -1;
        } else {
            print_ssh_error(session, "libssh2_userauth_list()");
            return -1;
        }
    }

    /* Sprawdzenie czy serwer obsluguje metode "publickey". */
    if (strstr(auth_list, "publickey") != NULL) {

        /*
         * Uwierzytelnianie za pomoca klucza publicznego.
         * cd->username - nazwa uzytkownika
         * pubkey - sciezka do klucza publicznego
         * privkey - sciezka do klucza prywatnego
         * passphrase - haslo zabezpieczajace dostep do klucza prywatnego
         */
        err = libssh2_userauth_publickey_fromfile(session, cd->username,
                pubkey, privkey, passphrase);
        if (err == 0) {
            return 0;
        }
    }

    /* Sprawdzenie czy serwer obsluguje metode "password". */
    if (strstr(auth_list, "password") == NULL) {
        fprintf(stderr, "Publickey method failed and password method is "
                "not supported by server.\n");
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
