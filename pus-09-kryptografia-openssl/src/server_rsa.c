/*
 * Data:                2009-05-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc server_rsa.c -lcrypto -o server_rsa
 * Uruchamianie:        $ ./server_rsa <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

int main(int argc, char** argv) {

    int             sockfd; /* Deskryptor gniazda. */
    int             retval; /* Wartosc zwracana przez funkcje. */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez recvfrom(): */
    char            buff[256];

    unsigned int    buff_len, message_len, sig_len;

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];

    FILE            *file;
    RSA             *public_key;


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /*                     RSA SIGNATURE VERIFICATION (part 1)                    */
    /******************************************************************************/

    /* Otworzenie pliku do odczytu: */
    file = fopen("public.key", "r");
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

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr     =       htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port            =       htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);

    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(sockfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is waiting for UDP datagram...\n");

    client_addr_len = sizeof(client_addr);

    /* Oczekiwanie na dane od klienta: */
    retval = recvfrom(
                 sockfd,
                 buff, sizeof(buff),
                 0,
                 (struct sockaddr*)&client_addr, &client_addr_len
             );
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }

    buff_len = retval;

    fprintf(stdout, "UDP datagram received from %s:%d.\n",
            inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
            ntohs(client_addr.sin_port)
           );

    /******************************************************************************/
    /*                     RSA SIGNATURE VERIFICATION (part 2)                    */
    /******************************************************************************/
    fprintf(stdout, "Signature verification...\n");

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /* Zaladowanie nazw algorytmow do pamieci: */
    OpenSSL_add_all_ciphers();

    /* Wczytanie klucza publicznego z pliku: */
    public_key = PEM_read_RSAPublicKey(file, NULL, NULL, NULL);
    if (!public_key) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie pliku: */
    fclose(file);

    /* Rozmiar podpisu cyfrowego: */
    sig_len = RSA_size(public_key);
    /* Rozmiar wiadomosci w buforze = rozmiar danych w buforze - rozmiar podpisu: */
    message_len = buff_len - sig_len;

    /* Weryfikacja podpisu cyfrowego za pomoca klucza publicznego: */
    retval = RSA_verify(NID_sha1, (unsigned char*)buff, message_len,
                        (unsigned char*)buff + message_len, sig_len, public_key);
    if (retval == 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "Signature is valid.\n");
    }

    /* Usuniecie struktury i klucza z pamieci: */
    RSA_free(public_key);

    /* Usuniecie nazw algorytmow z pamieci: */
    EVP_cleanup();

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();
    /******************************************************************************/

    fprintf(stdout, "Message: ");
    fwrite(buff, sizeof(char), message_len, stdout);
    fprintf(stdout, "\n");

    close(sockfd);
    exit(EXIT_SUCCESS);
}
