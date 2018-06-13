/*
 * Data:                2009-02-10
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc client.c -o client
 * Uruchamianie:        $ ./client <adres IP> <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <errno.h>
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
    memset(hmac_mac, 0, 256);
    unsigned int hmac_mac_len;

    ERR_load_crypto_strings();
    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname("md5");

    HMAC_CTX_init(hmac_ctx);
    HMAC_Init(hmac_ctx, key, 16, md);

    HMAC_Update(hmac_ctx, message, strlen(message));

    HMAC_Final(hmac_ctx, hmac_mac, &hmac_mac_len);

    fprintf(stdout, "HMAC_mac: ", hmac_mac);
    print_hex(hmac_mac, hmac_mac_len);
    fprintf(stdout, "HMAC_mac_len: %d\n", hmac_mac_len);

    memcpy(buff, hmac_mac, 16);
    memcpy(buff + 16, message, strlen(message));

    HMAC_CTX_cleanup(hmac_ctx);
    ERR_free_strings();
    EVP_cleanup();
}

void encrypt_cipher(char *buff, char *key, char *message) {
    EVP_CIPHER_CTX *ctx;
    const EVP_CIPHER* cipher;
    ERR_load_crypto_strings();
    ctx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
    EVP_CIPHER_CTX_init(ctx);
    cipher = EVP_aes_128_ecb();
    int retval = EVP_EncryptInit_ex(ctx, cipher, NULL, key, NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    int buff_len = strlen(message) + 20;
    EVP_CIPHER_CTX_set_padding(ctx, 1);
    printf("Message to encrypt: %s\n", message);
    retval = EVP_EncryptUpdate(ctx, buff, &buff_len, message, strlen(message));

    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    int  tmp;
    retval = EVP_EncryptFinal_ex(ctx, buff + buff_len, &tmp);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    buff[buff_len + tmp] = '\0';

    EVP_CIPHER_CTX_cleanup(ctx);
    ERR_free_strings();

    printf("Cipher: ");
    print_hex(buff, buff_len + tmp);
}


int main(int argc, char** argv) {

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t       addr_len;               /* Rozmiar struktury w bajtach. */

    char            message[256] = "Laboratorium PUS.";
    char hmac_message[256];
    memset(hmac_message, 0, 256);
    char cipher_message[256];
    memset(cipher_message, 0, 256);

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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

    fprintf(stdout, "Sending message to %s.\n", argv[1]);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */

    hmac(hmac_message, key, message);
    encrypt_cipher(cipher_message, key, hmac_message);


    retval = (int) sendto(
                     sockfd,
                     cipher_message, strlen(cipher_message),
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
