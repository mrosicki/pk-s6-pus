/*
 * Data:                2009-05-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc cipher_bio.c -lcrypto -o cipher_bio
 * Uruchamianie:        $ ./cipher_bio
 *                      $ ./cipher_bio < NAZWA_PLIKU
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bio.h>

int main(int argc, char **argv) {

    /* Wiadomosc do zaszfrowania: */
    char plaintext[80];

    /* Rozmiar tekstu jawnego: */
    int plaintext_len;

    /* Klucz: */
    unsigned char key[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                           0x00,0x01,0x02,0x03,0x04,0x05
                          };

    /* Wektor inicjalizacyjny: */
    unsigned char iv[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
                          0x00,0x01,0x02,0x03,0x04,0x05
                         };

    BIO *bio_encrypt, *bio_base64, *bio_stdout;

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /* Pobranie maksymalnie 64 znakow ze standardowego wejscia: */
    if (fgets(plaintext, 64, stdin) == NULL) {
        fprintf(stderr, "fgets() failed!\n");
        exit(EXIT_FAILURE);
    }

    plaintext_len = strlen(plaintext);

    fprintf(stdout, "Ciphertext: ");

    /* Utworzenie BIO filtrujacego (do szyfrowania): */
    bio_encrypt = BIO_new(BIO_f_cipher());
    /*
     * Konfiguracja algorytmu, klucza i wektora inicjalizacyjnego dla operacji
     * szyfrowania (statni argument rowny 1):
     */
    BIO_set_cipher(bio_encrypt, EVP_aes_128_cbc(), key, iv, 1);

    /* Utworzenie BIO filtrujacego (do kodowania BASE64): */
    bio_base64 = BIO_new(BIO_f_base64());

    /* Utworzenie BIO do przesylania danych na standardowe wyjscie: */
    bio_stdout = BIO_new_fp(stdout, BIO_NOCLOSE);

    /* Powiazanie BIO (utworzenie lancucha encrypt-base64-stdout): */
    BIO_push(bio_encrypt, bio_base64);
    BIO_push(bio_base64, bio_stdout);

    /* Wysylanie tekstu jawnego do lancucha BIO: */
    if (BIO_write(bio_encrypt, plaintext, plaintext_len) <= 0) {
        if (!BIO_should_retry(bio_encrypt)) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    }

    /* Oproznienie wewnetrznego bufora: */
    if (BIO_flush(bio_encrypt) != 1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Zwolnienie lancucha BIO: */
    BIO_free_all(bio_encrypt);

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    /*
     * Weryfikacja:
     * openssl enc -aes-128-cbc -K 00010203040506070809000102030405 -iv 00010203040506070809000102030405 -a -in plaintext
     */
    exit(EXIT_SUCCESS);
}
