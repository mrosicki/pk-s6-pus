/*
 * Data:                2009-06-26
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include "libcommon.h"

/*
 * Wypisuje na standardowe wyjscie bledow ostatni blad biblioteki libssh2.
 * Opis bledu jest poprzedzony prefiksem.
 */
void print_ssh_error(LIBSSH2_SESSION *session, const char *prefix) {

    char    *errmsg;
    int     errmsg_len;

    libssh2_session_last_error(session, &errmsg, &errmsg_len, 0);
    fprintf(stderr, "%s: %s\n", prefix, errmsg);
}

/*
 * Wypisuje na standardowe wyjscie bledow ostatni blad biblioteki libssh2
 * zwiazany z podsystemem "sftp".
 * Opis bledu jest poprzedzony prefiksem.
 */
void print_sftp_error(LIBSSH2_SESSION *session, LIBSSH2_SFTP *sftp,
                      const char *prefix) {

    static const char *errmsg;

    if (libssh2_session_last_errno(session) == LIBSSH2_ERROR_SFTP_PROTOCOL) {

        switch (libssh2_sftp_last_error(sftp)) {
        case LIBSSH2_FX_OK:
            errmsg = "LIBSSH2_FX_OK";
            break;
        case LIBSSH2_FX_EOF:
            errmsg = "LIBSSH2_FX_EOF";
            break;
        case LIBSSH2_FX_NO_SUCH_FILE:
            errmsg = "LIBSSH2_FX_NO_SUCH_FILE";
            break;
        case LIBSSH2_FX_PERMISSION_DENIED:
            errmsg = "LIBSSH2_FX_PERMISSION_DENIED";
            break;
        case LIBSSH2_FX_FAILURE:
            errmsg = "LIBSSH2_FX_FAILURE";
            break;
        case LIBSSH2_FX_BAD_MESSAGE:
            errmsg = "LIBSSH2_FX_BAD_MESSAGE";
            break;
        case LIBSSH2_FX_NO_CONNECTION:
            errmsg = "LIBSSH2_FX_NO_CONNECTION";
            break;
        case LIBSSH2_FX_CONNECTION_LOST:
            errmsg = "LIBSSH2_FX_CONNECTION_LOST";
            break;
        case LIBSSH2_FX_OP_UNSUPPORTED:
            errmsg = "LIBSSH2_FX_OP_UNSUPPORTED";
            break;
        case LIBSSH2_FX_INVALID_HANDLE:
            errmsg = "LIBSSH2_FX_INVALID_HANDLE";
            break;
        case LIBSSH2_FX_NO_SUCH_PATH:
            errmsg = "LIBSSH2_FX_NO_SUCH_PATH";
            break;
        case LIBSSH2_FX_FILE_ALREADY_EXISTS:
            errmsg = "LIBSSH2_FX_FILE_ALREADY_EXISTS";
            break;
        case LIBSSH2_FX_WRITE_PROTECT:
            errmsg = "LIBSSH2_FX_WRITE_PROTECT";
            break;
        case LIBSSH2_FX_NO_MEDIA:
            errmsg = "LIBSSH2_FX_NO_MEDIA";
            break;
        case LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM:
            errmsg = "LIBSSH2_FX_NO_SPACE_ON_FILESYSTEM";
            break;
        case LIBSSH2_FX_QUOTA_EXCEEDED:
            errmsg = "LIBSSH2_FX_QUOTA_EXCEEDED";
            break;
        case LIBSSH2_FX_UNKNOWN_PRINCIPAL:
            errmsg = "LIBSSH2_FX_UNKNOWN_PRINCIPAL";
            break;
        case LIBSSH2_FX_LOCK_CONFLICT:
            errmsg = "LIBSSH2_FX_LOCK_CONFLICT";
            break;
        case LIBSSH2_FX_DIR_NOT_EMPTY:
            errmsg = "LIBSSH2_FX_DIR_NOT_EMPTY";
            break;
        case LIBSSH2_FX_NOT_A_DIRECTORY:
            errmsg = "LIBSSH2_FX_NOT_A_DIRECTORY";
            break;
        case LIBSSH2_FX_INVALID_FILENAME:
            errmsg = "LIBSSH2_FX_INVALID_FILENAME";
            break;
        case LIBSSH2_FX_LINK_LOOP:
            errmsg = "LIBSSH2_FX_LINK_LOOP";
            break;
        default:
            errmsg = "OTHER";
        }

        fprintf(stderr, "%s: %s\n", prefix, errmsg);

    }
}

/*
 * Ustanawia polaczenie TCP i zwraca deskryptor dla gniazda polaczonego lub -1
 * w przypadku bledu.
 */
int establish_tcp_connection(struct connection_data *cd) {

    int                     err;
    int                     sockfd;
    struct sockaddr_in      sockaddr;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        return -1;
    }

    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(cd->port);
    sockaddr.sin_addr = cd->address;

    err = connect(sockfd, (const struct sockaddr*)&sockaddr,
                  sizeof(struct sockaddr_in));

    if (err == -1) {
        perror("connect()");
        return -1;
    }

    return sockfd;
}


/*
 * Funkcja odpowiedzialna za uwierzytelnianie serwera na podstawie cyfrowego
 * odcisku palca.
 */
int authenticate_server(LIBSSH2_SESSION *session) {

    int             i;
    const char      *hash;
    char            accept[256];
    char            *charptr;

    hash = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_MD5);
    if (hash == NULL) {
        fprintf(stderr, "Session has not ben started up.\n");
        return -1;
    }

    fprintf(stdout, "Fingerprint: ");
    for (i = 0; i < 15; i++) {
        fprintf(stdout, "%02x:", (unsigned char)hash[i]);
    }
    fprintf(stdout, "%02x\n", (unsigned char)hash[i]);

    fprintf(stdout, "Accept? (y/n): ");
    charptr = fgets(accept, 256, stdin);
    if (strncmp(accept, "y", 1) != 0 && strncmp(accept, "Y", 1) != 0)  {
        return -1;
    }

    return 0;
}


/*
 * Funkcja pobiera haslo uzytkownika.
 * prompt - zapytanie jakie zostanie wyswietlone przed mozliwoscia podania hasla
 * buf - bufor na haslo; programista jest odpowiedzialny za alokacje pamieci
 * buf_len - rozmiar bufora
 *
 * Jezeli haslo nie bedzie wiecej potrzebne, zaleca sie wyzerowanie bufora.
 */
int get_password(const char *prompt, char *buf, unsigned int buf_len) {

    struct termios          term, oterm;
    sigset_t                set, oset;
    int                     in, out;
    char                    ch, *p, *end;
    ssize_t                 r;
    int                     err;

    if (buf_len == 0) {
        return -1;
    }

    in = out = open("/dev/tty", O_RDWR);
    if (in == -1) {
        in = STDIN_FILENO;
        out = STDOUT_FILENO;
    }

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTSTP);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, &oset);

    if (in != STDIN_FILENO && tcgetattr(in, &oterm) == 0) {
        memcpy(&term, &oterm, sizeof(term));
        term.c_lflag &= ~(ECHO | ECHONL);
        tcsetattr(in, TCSAFLUSH, &term);
    } else {
        memset(&term, 0, sizeof(term));
        term.c_lflag |= ECHO;
        memset(&oterm, 0, sizeof(oterm));
        oterm.c_lflag |= ECHO;
    }

    err = write(out, prompt, strlen(prompt));

    end = buf + buf_len - 1;
    /*
     * ICANON jest domyslnie wlaczony. Oznacza to ze nie czytamy znak
     * po znaku w trakcie pisania, ale dopiero gdy wczytana zostala
     * pelna linia. Dzieku temu mozna uzywac backspace.
     */
    for (p = buf;
            (r = read(in, &ch, 1)) == 1 && ch != '\n' && ch != '\r' && p < end;
            p++
        ) {
        *p = ch;
    }

    *p = '\0';

    if (!(term.c_lflag & ECHO)) {
        err = write(out, "\n", 1);
    }

    if (memcmp(&term, &oterm, sizeof(term)) != 0) {
        tcsetattr(in, TCSAFLUSH, &oterm);
    }

    sigprocmask(SIG_SETMASK, &oset, NULL);

    if (in != STDIN_FILENO) {
        close(in);
    }

    return (r == -1) ? -1 : 0;
}

/*
 * Funkcja odpowiedzialna za parsowanie argumentow wywolania programu.
 * Zwraca wskaznik na dynamicznie zaalokowana strukture connection_data lub NULL
 * w przypadku bledu.
 *
 * struct connection_data {
 *      char            *username; //Nazwa uzytkownika
 *      struct in_addr  address;   //adres IPv4
 *      unsigned short  port;      //numer portu, domyslnie: 22
 * };
 *
 * Programista jest odpowiedzialny za zwolnienie pamieci za pomoca funkcji
 * free_connection_data().
 *
 * Parametr required jest maska bitowa okreslajaca jakie argumenty sa wymagane:
 * CD_ADDRESS  - nazwa/adres IPv4 hosta
 * CD_PORT     - numer portu serwera
 * CD_USERNAME - nazwa uzytkownika
 *
 * np.:
 * required = CD_ADDRESS | CD_USERNAME;
 */
struct connection_data* parse_connection_data(int argc, char **argv, int required) {

    char                    **argvp, *tmp;
    const char              *arg, *ptr, *prev;
    int                     i, err;
    unsigned long           port;
    struct connection_data  *cd;
    struct addrinfo         hints;
    struct addrinfo         *result;

    if (argc < 2) {
        fprintf(stderr,
                "Invocation: %s [OPTIONS] <USER NAME>@<HOST>\n\n"
                "OPTIONS:\n"
                "  -p <port number>\n"
                "  Port to connect to remote host; default: 22\n",
                argv[0]);

        exit(EXIT_FAILURE);
    }

    cd = (struct connection_data*)malloc(sizeof(struct connection_data));
    if (!cd) {
        fprintf(stderr, "malloc() failed!\n");
        goto free;
    }

    cd->port = 22;

    argvp = argv;
    while (*argvp != NULL) {

        arg = *argvp;

        if ((ptr = strstr(arg, "-p")) && (arg == ptr)) {

            if (strlen(arg) != 2) {

                ptr += 2;
                tmp = (char*)ptr;
                i = 0;
                while (tmp != NULL) {
                    if (!isdigit(*tmp++)) {
                        i = 5;
                        break;
                    }
                    ++i;
                }

                if (i == 0 || i > 5) {
                    fprintf(stderr, "Invalid argument: %s.\n", arg);
                    goto free;
                }

                port = strtol(ptr, NULL, 10);
                if (port == 0 || port > 65535) {
                    fprintf(stderr, "Invalid port number: %lu.\n", port);
                    goto free;
                }

                cd->port = (unsigned short)port;
                required &= ~CD_PORT;

            } else {

                prev = arg;
                arg = *(++argvp);
                if (arg == NULL) {
                    fprintf(stderr, "Invalid argument: %s.\n", prev);
                    goto free;
                }

                tmp = (char*)arg;
                i = 0;
                while (tmp != NULL) {
                    if (!isdigit(*tmp++)) {
                        i = 5;
                        break;
                    }
                    ++i;
                }

                if (i == 0 || i > 5) {
                    fprintf(stderr, "Invalid argument: %s %s.\n",
                            prev, arg);
                    goto free;
                }

                port = strtol(arg, NULL, 10);
                if (port == 0 || port > 65535) {
                    fprintf(stderr, "Invalid port number: %lu.\n", port);
                    goto free;
                }

                cd->port = (unsigned short)port;
                required &= ~CD_PORT;
            }

        } else if ((ptr = strstr(arg, "@"))) {

            if (ptr == arg) {
                fprintf(stderr, "Invalid argument: %s.\n", arg);
                goto free;
            }

            cd->username = (char*)malloc(ptr - arg + 1);
            memcpy(cd->username, arg, ptr - arg);
            cd->username[ptr - arg] = '\0';
            required &= ~CD_USERNAME;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family                 =       AF_INET;
            hints.ai_socktype               =       SOCK_STREAM;
            hints.ai_protocol               =       IPPROTO_TCP;

            err = getaddrinfo(++ptr, NULL, &hints, &result);
            if (err != 0) {
                fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(err));
                goto free;
            }

            if (result != NULL) {
                cd->address = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
                required &= ~CD_ADDRESS;
                freeaddrinfo(result);
            } else {
                fprintf(stderr, "Invalid argument: %s.\n", arg);
                goto free;
            }
        }

        ++argvp;

    }

    if (required) {
        fprintf(stderr,
                "Invocation: %s [OPTIONS] <USER NAME>@<HOST>\n\n"
                "OPTIONS:\n"
                "  -p <port number>\n"
                "  Port to connect to on the remote host; default: 22\n",
                argv[0]);

        goto free;
    }

    return cd;

free:
    free_connection_data(cd);
    return NULL;
}

/*
 * Funkcja odpowiedzialna za zwolnienie pamieci zaalokowanej na rzecz struktury
 * connection_data.
 */
void free_connection_data(struct connection_data* cd) {

    if (cd != NULL) {
        if (cd->username != NULL) {
            free((void*)cd->username);
            cd->username = NULL;
        }

        free(cd);
    }
}
