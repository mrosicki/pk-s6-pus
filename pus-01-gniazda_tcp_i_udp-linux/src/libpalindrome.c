/*
 * Data:                2009-03-16
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc -c libpalindrome.c -o libpalindrome.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
 * Funkcja sprawdza czy dane w buforze wskazywanym przez buff sa palindromem
 * liczbowym. Funkcja ignoruje biale znaki i wiodace zera.
 *
 * Parametry:
 * buff - wskaznik na bufor zawierajacy dane odebrane za pomoca funkcji read(),
 *        recv() lub recvfrom()
 *
 * buff_len - rozmiar danych w buforze (nie rozmiar bufora); jest to liczba
 *            bajtow zwrocona przez funkcje read(), recv() lub recvfrom()
 *
 * Wartosc zwracana:
 * -1 - dane w buforze zawieraja znaki, ktore nie sa cyframi lub znakami bialymi
 *  0 - dane w buforze nie sa palindromem liczbowym
 *  1 - dane w buforze sa palindromem liczbowym*
 *
 */
int is_palindrome(char* buff, int buff_len) {

    /*
     * Wskaznik na bufor, w ktorym przechowywana bedzie liczba bez bialych
     * znakow i wiodacych zer:
     */
    char* tmp;

    int i, j;
    char* head, *tail;

    /* Alokacja pamieci dla bufora: */
    tmp = (char*)malloc(buff_len);
    if (tmp == NULL) {
        fprintf(stderr, "malloc() failed!\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Do bufora 'tmp' kopiowane sa tylko cyfry (bez bialych znakow).
     * Wystapienie litery jest bledem:
     */
    j = 0;
    for (i = 0; i < buff_len; ++i) {
        if (isdigit(buff[i])) {
            tmp[j++] = buff[i];
            fprintf(stdout,"%c", buff[i]);
            fflush(stdout);
        } else if (isalpha(buff[i])) {
            free(tmp);
            return -1; /* String nie jest liczba. */
        }
    }

    /* Usuniecie wiodacych zer: */
    for (i = 0; i < j; ++i) {
        if (tmp[i] != '0') {
            break;
        }
    }

    head = &tmp[i];
    tail = &tmp[j - 1];

    do {
        if (*head++ != *tail--) {
            free(tmp);
            return 0; /* Liczba nie jest palindromem. */
        }
    } while (head <= tail);

    free(tmp);
    return 1; /* Liczba jest palindromem. */
}
