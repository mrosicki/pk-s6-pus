/*
 * Data:                2009-06-26
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 */

#ifndef _LIB_COMMON
#define _LIB_COMMON

#include <netinet/in.h>
#include <libssh2.h>
#include <libssh2_sftp.h>
/*
 * Struktura przechowujaca dane wykorzystywane podczas nawiazywania polaczenia
 * z serwerem SSH.
 */
struct connection_data {
    char            *username;
    struct in_addr  address;
    unsigned short  port;
};

/*
 * Okresla nazwy argumentow wywolania programu.
 * Nazwy sa stosowane przez funkcje parse_connection_data() w celu okreslenia
 * jakie argumenty sa wymagane, a jakie opcjonalne.
 */
enum connection_data_opt {
    CD_ADDRESS = 1,
    CD_PORT = 2,
    CD_USERNAME = 4
};

/* Opis funkcji znajduje sie w pliku "libcommon.c": */
int establish_tcp_connection(struct connection_data *cd);
int authenticate_server(LIBSSH2_SESSION *session);
void print_ssh_error(LIBSSH2_SESSION    *session, const char *prefix);
void print_sftp_error(LIBSSH2_SESSION *session, LIBSSH2_SFTP *sftp, const char *prefix);

int get_password(const char *prompt, char *buf, unsigned int buf_len);
struct connection_data* parse_connection_data(int argc, char **argv, int required);
void free_connection_data(struct connection_data *cd);

#endif /* _LIB_CD */
