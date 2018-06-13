/*
 * Data:                2009-02-10
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc server.c -o server
 * Uruchamianie:        $ ./server <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>
#include <openssl/hmac.h>
#include <openssl/err.h>

/* Klucz: */
unsigned char key[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                       0x00,0x01,0x02,0x03,0x04,0x05
};

/* Wektor inicjalizacyjny: */
unsigned char iv[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                      0x00,0x01,0x02,0x03,0x04,0x05
};

void print_hex(const char * message, int len) {
    int i;
    for (i= 0; i < len; i++) {
        fprintf(stdout, "%02x", (unsigned char)message[i]);
    }
    fprintf(stdout, "\n");
}

void hmac(char * buff, char * key,  const unsigned char * message) {
    HMAC_CTX *hmac_ctx;
    hmac_ctx = (HMAC_CTX *) malloc(sizeof(HMAC_CTX));
    const EVP_MD *md;
    unsigned char hmac_mac[256];
    unsigned int hmac_mac_len;

    ERR_load_crypto_strings();
    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname("md5");

    HMAC_CTX_init(hmac_ctx);
    HMAC_Init(hmac_ctx, key, 16, md);

    HMAC_Update(hmac_ctx, message, strlen(message));

    HMAC_Final(hmac_ctx, hmac_mac, &hmac_mac_len);

    memcpy(buff, hmac_mac, 16);

    HMAC_CTX_cleanup(hmac_ctx);
    ERR_free_strings();
    EVP_cleanup();
}

void decrypt_cipher(char *buff, char *key, char *message) {
    EVP_CIPHER_CTX *ctx;
    const EVP_CIPHER* cipher;
    ERR_load_crypto_strings();
    ctx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    EVP_CIPHER_CTX_init(ctx);
    cipher = EVP_aes_128_ecb();
    int retval = EVP_DecryptInit_ex(ctx, cipher, NULL, key, NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    int buff_len = strlen(message);

    EVP_CIPHER_CTX_set_padding(ctx, 1);
    retval = EVP_DecryptUpdate(ctx, buff, &buff_len, message, strlen(message));

    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    int tmp;
    retval = EVP_DecryptFinal_ex(ctx, (unsigned char*)buff + buff_len, &tmp);
    buff[buff_len + tmp] = '\0';
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    EVP_CIPHER_CTX_cleanup(ctx);
    ERR_free_strings();
}


void get_hmac_from_message(char * buff, char *message) {
    memset(buff, 0, 16);
    strncpy(buff, message, 16);
}

void remove_hmac_from_message(char * buff, char *message) {
    int message_len = strlen(message) - 16 + 1;
    memset(buff, 0, message_len);
    strncpy(buff, message + 16, message_len);
}

int main(int argc, char** argv) {

    int             sockfd; /* Deskryptor gniazda. */
    int             retval; /* Wartosc zwracana przez funkcje. */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez recvfrom(): */
    char            cipher_buff[256];
    memset(cipher_buff, 0, 256);

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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

    retval = (int) recvfrom(
                     sockfd,
                     cipher_buff, sizeof(cipher_buff),
                     0,
                     (struct sockaddr*)&client_addr, &client_addr_len
                 );
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "UDP datagram received from %s:%d.\n",
            inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
            ntohs(client_addr.sin_port)
           );

    char plain_text_buff[256];
    char hmac_buff[256];
    char message_buff[256];
    char hmac_calculated_buff[256];

    memset(plain_text_buff, 0, 256);
    memset(hmac_buff, 0, 256);
    memset(message_buff, 0, 256);
    memset(hmac_calculated_buff, 0, 256);

    decrypt_cipher(plain_text_buff, key, cipher_buff);
    get_hmac_from_message(hmac_buff, plain_text_buff);
    remove_hmac_from_message(message_buff, plain_text_buff);
    hmac(hmac_calculated_buff, key, message_buff);

    fprintf(stdout, "Cipher: ");
    print_hex(cipher_buff, strlen(cipher_buff));

    fprintf(stdout, "Plain text: ");
    fwrite(plain_text_buff, sizeof(char), retval, stdout);
    fprintf(stdout, "\n");

    fprintf(stdout, "Hmac received: ");
    print_hex(hmac_buff, strlen(hmac_buff));

    fprintf(stdout, "Hmac calculated: ");
    print_hex(hmac_calculated_buff, strlen(hmac_calculated_buff));

    fprintf(stdout, "Decrypted message: ");
    fwrite(message_buff, sizeof(char), retval, stdout);
    fprintf(stdout, "\n");

    close(sockfd);
    exit(EXIT_SUCCESS);
}
