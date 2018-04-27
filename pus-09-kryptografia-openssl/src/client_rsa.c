/*
 * Data:                2009-05-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc client_rsa.c -lcrypto -o client_rsa
 * Uruchamianie:        $ ./client_rsa <adres IP> <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

int main(int argc, char** argv) {

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t       addr_len;               /* Rozmiar struktury w bajtach. */
    FILE            *file;
    RSA             *private_key;
    unsigned int    message_len, sig_len;

    char            buff[256] = "Laboratorium PUS.";


    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /*                            RSA SIGNING (patr 1)                            */
    /******************************************************************************/

    /* Otworzenie pliku do odczytu: */
    file = fopen("private.key", "r");
    if (file == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }
    /******************************************************************************/

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    remote_addr.sin_family = AF_INET;

    /* Konwersja adresu IP z postaci kropkowo-dziesietnej: */
    retval = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    remote_addr.sin_port = htons(atoi(argv[2])); /* Numer portu. */
    addr_len = sizeof(remote_addr); /* Rozmiar struktury adresowej w bajtach. */

    /******************************************************************************/
    /*                            RSA SIGNING (patr 2)                            */
    /******************************************************************************/
    fprintf(stdout, "Signing message...\n");

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /* Zaladowanie nazw algorytmow do pamieci: */
    OpenSSL_add_all_ciphers();

    /* Wczytanie klucza prywatnego z pliku: */
    private_key = PEM_read_RSAPrivateKey(file, NULL, NULL, NULL);
    if (!private_key) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie pliku: */
    fclose(file);

    /* Rozmiar wiadomosci w buforze: */
    message_len = strlen(buff);

    /*
     * Utworzenie podpisu cyfrowego. Podpis jest umieszczany w buforze 'buff' za
     * wiadomoscia:
     */
    retval = RSA_sign(NID_sha1, (unsigned char*)buff, message_len,
                      (unsigned char*)buff + message_len, &sig_len, private_key);
    if (retval == 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Usuniecie struktury i klucza z pamieci: */
    RSA_free(private_key);

    /* Usuniecie nazw algorytmow z pamieci: */
    EVP_cleanup();

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    fprintf(stdout, "Signature size: %u bytes.\n", sig_len);
    /******************************************************************************/

    fprintf(stdout, "Sending signed message to %s.\n", argv[1]);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
                 sockfd,
                 buff, message_len + sig_len,
                 0,
                 (struct sockaddr*)&remote_addr, addr_len
             );

    if (retval == -1) {
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
