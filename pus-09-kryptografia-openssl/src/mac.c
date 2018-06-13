/*
 * Data:                2009-05-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc digest.c -lcrypto -o digest
 * Uruchamianie:        $ ./digest <nazwa funkcji>
 *                      $ ./digest <nazwa funkcji> < NAZWA_PLIKU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

int main(int argc, char **argv) {

    /* Wartosc zwracana przez funkcje: */
    int retval;

    int i;

    /* Wiadomosc: */
    char message[64];

    /* Skrot wiadomosci: */
    unsigned char digest[EVP_MAX_MD_SIZE];

    /* Rozmiar tekstu i szyfrogramu: */
    unsigned int message_len, digest_len;

    /* Kontekst: */
    HMAC_CTX *hctx;

    const EVP_MD* md;

    unsigned char key[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                           0x00,0x01,0x02,0x03,0x04,0x05
                          };

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <DIGEST NAME>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /*
     * Zaladowanie nazw funkcji skrotu do pamieci.
     * Wymagane przez EVP_get_digestbyname():
     */
    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname(argv[1]);
    if (!md) {
        fprintf(stderr, "Unknown message digest: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Pobranie maksymalnie 64 znakow ze standardowego wejscia: */
    if (fgets(message, 64, stdin) == NULL) {
        fprintf(stderr, "fgets() failed!\n");
        exit(EXIT_FAILURE);
    }

    message_len = strlen(message);

    /* Alokacja pamieci dla kontekstu: */
    hctx = (HMAC_CTX*)malloc(sizeof(HMAC_CTX));
    /* Inicjalizacja kontekstu: */
    HMAC_CTX_init(hctx);
    

    /* Parametry funkcji skrotu: */
    fprintf(stdout, "Digest parameters:\n");
    fprintf(stdout, "Block size: %d bits\n", EVP_MD_block_size(md));
    fprintf(stdout, "Digest size: %d bytes\n\n", EVP_MD_size(md));

    /* Konfiguracja kontekstu: */
    retval = HMAC_Init_ex(hctx,key,16,md,NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Obliczenie skrotu: */
    retval = HMAC_Update(hctx,message,message_len);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*Zapisanie skrotu w buforze 'digest': */
    retval = HMAC_Final(hctx,digest,&digest_len);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*
     * Usuwa wszystkie informacje z kontekstu i zwalnia pamiec zwiazana
     * z kontekstem:
     */
    HMAC_CTX_cleanup(hctx);

    /* Usuniecie nazw funkcji skrotu z pamieci. */
    EVP_cleanup();

    fprintf(stdout, "Digest (hex): ");
    for (i = 0; i < digest_len; i++) {
        fprintf(stdout, "%02x", digest[i]);
    }

    fprintf(stdout, "\n");

    if (hctx) {
        free(hctx);
    }

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    exit(EXIT_SUCCESS);
}
