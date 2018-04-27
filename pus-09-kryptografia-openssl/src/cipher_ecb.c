/*
 * Data:                2009-05-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc cipher_ecb.c -lcrypto -o cipher_ecb
 * Uruchamianie:        $ ./cipher_ecb
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>

int main(int argc, char **argv) {

    /* Wartosc zwracana przez funkcje: */
    int retval;

    int i, tmp;

    /* Wiadomosc do zaszfrowania: */
    char plaintext[80] = "MESSAGE_MESSAGE_MESSAGE_MESSAGE_";

    /* Bufor na szyfrogram: */
    unsigned char ciphertext[80];

    /* Rozmiar tekstu i szyfrogramu: */
    int plaintext_len, ciphertext_len;

    /* Rozmiar bloku, na ktorym operuje algorytm: */
    int block_size;

    /*
     * == 0 - padding PKCS nie bedzie stosowany
     * != 0 - padding PKCS bedzie stosowany
     */
    int padding;

    /* Klucz i wektor inicjalizacyjny sa stalymi, aby wyniki byly przewidywalne. */

    /* Klucz: */
    unsigned char key[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                           0x00,0x01,0x02,0x03,0x04,0x05
                          };

    /* Kontekst: */
    EVP_CIPHER_CTX *ctx;

    const EVP_CIPHER* cipher;

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PADDING>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    padding = atoi(argv[1]);

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /* Alokacja pamieci dla kontekstu: */
    ctx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));

    /* Inicjalizacja kontekstu: */
    EVP_CIPHER_CTX_init(ctx);

    /*
     * Parametry algorytmu AES dla trybu ECB i klucza o rozmiarze 128-bitow.
     * Liste funkcji typu "EVP_aes_128_ecb()" mozna uzyskac z pliku <openssl/evp.h>.
     * Strony podrecznika systemowego nie sa kompletne.
     */
    cipher = EVP_aes_128_ecb();
    fprintf(stdout, "Cipher parameters:\n");
    fprintf(stdout, "Block size: %d\n", block_size = EVP_CIPHER_block_size(cipher));
    fprintf(stdout, "Key length: %d\n\n", EVP_CIPHER_key_length(cipher));

    fprintf(stdout, "Encrypting...\n\n");
    /* Konfiguracja kontekstu dla szyfrowania: */
    retval = EVP_EncryptInit_ex(ctx, cipher, NULL, key, NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*
     * Domyslnie OpenSSL stosuje padding. 0 - padding nie bedzie stosowany.
     * Funkcja moze byc wywolana tylko po konfiguracji kontekstu
     * dla szyfrowania/deszyfrowania (odzielnie dla kazdej operacji).
     */
    EVP_CIPHER_CTX_set_padding(ctx, padding);

    plaintext_len = strlen(plaintext);

    /* Szyfrowanie: */
    retval = EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len,
                               (unsigned char*)plaintext, plaintext_len);

    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    retval = EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &tmp);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*
     * Usuwa wszystkie informacje z kontekstu i zwalnia pamiec zwiazana
     * z kontekstem:
     */
    EVP_CIPHER_CTX_cleanup(ctx);

    ciphertext_len += tmp;

    fprintf(stdout, "Plaintext (hex):\n");
    for (i= 0; i < plaintext_len;) {
        fprintf(stdout, "%02x", (unsigned char)plaintext[i]);

        i++;
        /* Wypisanie separatora oddzielajacego bloki: */
        if ((i % block_size == 0) && (i != plaintext_len)) {
            fprintf(stdout, "-");
        }
    }

    fprintf(stdout, "\nCiphertext (hex):\n");
    for (i = 0; i < ciphertext_len;) {
        fprintf(stdout, "%02x", ciphertext[i]);

        i++;
        /* Wypisanie separatora oddzielajacego bloki: */
        if ((i % block_size == 0) && (i != ciphertext_len)) {
            fprintf(stdout, "-");
        }
    }

    fprintf(stdout, "\n\n");
    fprintf(stdout, "Plaintext length: %u\n", plaintext_len);
    fprintf(stdout, "Ciphertext length: %u\n\n", ciphertext_len);

    /* Wyzerowanie tekstu jawnego: */
    memset(plaintext, 0, 80);

    fprintf(stdout, "Decrypting...\n\n");
    /* Konfiguracja kontekstu dla odszyfrowywania: */
    retval = EVP_DecryptInit_ex(ctx, cipher, NULL, key, NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*
     * Domyslnie OpenSSL stosuje padding. 0 - padding nie bedzie stosowany.
     * Funkcja moze byc wywolana tylko po konfiguracji kontekstu
     * dla szyfrowania/deszyfrowania (odzielnie dla kazdej operacji).
     */
    EVP_CIPHER_CTX_set_padding(ctx, padding);

    /* Odszyfrowywanie: */
    retval = EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &plaintext_len,
                               ciphertext, ciphertext_len);

    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*
     * Prosze zwrocic uwage, ze rozmiar bufora 'plaintext' musi byc co najmniej o
     * rozmiar bloku wiekszy od dlugosci szyfrogramu (na padding):
     */
    retval = EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + plaintext_len, &tmp);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    plaintext_len += tmp;
    plaintext[plaintext_len] = '\0';
    fprintf(stdout, "Plaintext:\n%s\n", plaintext);

    EVP_CIPHER_CTX_cleanup(ctx);
    if (ctx) {
        free(ctx);
    }
    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    exit(EXIT_SUCCESS);
}
