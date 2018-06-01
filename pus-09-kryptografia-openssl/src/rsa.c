/*
 * Data:                2009-05-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc rsa.c -lcrypto -o rsa
 * Uruchamianie:        $ ./rsa
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

int main(int argc, char **argv) {

    /* Wartosc zwracana przez funkcje: */
    int retval;

    /* Wskaznik na plik: */
    FILE *file;

    /* Wskaznik na strukture przechowujaca pare kluczy RSA: */
    RSA *key_pair;

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /*
     * Inicjalizacja generatora liczb pseudolosowych za pomoca pliku
     * /dev/urandom:
     */
    RAND_load_file("/dev/urandom", 1024);

    /* Wygenerowanie kluczy: */
    key_pair = RSA_generate_key(1024, RSA_F4, NULL, NULL);
    if (key_pair == NULL) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Weryfikacja poprawnosci kluczy: */
    retval = RSA_check_key(key_pair);
    if (retval <= 0) {
        if (retval == -1) {
            ERR_print_errors_fp(stderr);
        }
        fprintf(stderr, "Problem with keys. They should be regenerated.\n");
        exit(EXIT_FAILURE);
    }

    /* Otworzenie pliku do zapisu: */
    file = fopen("public.key", "w");
    if (file == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    /* Zapisanie klucza publicznego w formacie PEM (kodowanie Base64) do pliku: */
    retval = PEM_write_RSAPublicKey(file, key_pair);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    file = fopen("private.key", "w");
    if (file == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    /*
     * Zapisanie klucza prywatnego w formacie PEM do pliku.
     * Klucz jest szyfrowany za pomoca algorytmu AES.
     * Uzytkownik jest pytany o haslo, na podstawie ktorego zastanie wygenerowany
     * klucz dla szyfrowania symetrycznego.
     * Po zaszyfrowaniu klucza prywatnego RSA, jest on kodowany za pomoca Base64:
     */
    retval = PEM_write_RSAPrivateKey(file, key_pair, EVP_aes_256_cbc(),
                                     NULL, 0, NULL, NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    /* Zwolnienie pamieci: */
    RSA_free(key_pair);

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    exit(EXIT_SUCCESS);
}
