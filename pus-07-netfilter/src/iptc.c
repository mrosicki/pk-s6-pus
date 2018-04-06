/*
 * Data:                2009-04-15
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc iptc.c -o iptc -liptc
 * Uruchamianie:        $ ./iptc -h
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libiptc/libiptc.h>
/* Zdefiniowane operacje: */
enum opcode {
    NONE,
    DELETE_RULE,
    NEW_CHAIN
};

/* Regula - zawiera argumenty wywolania programu: */
struct rule {
    enum opcode     operation; /* Rodzaj operacji, np. '-D' (DELETE_RULE) */
    const char      *chain; /* Nazwa lancucha. */
    const char      *table; /* Nazwa tablicy ('-t <table>') */
    /*
     * Numer reguly (numeracja od 0). W programie iptables
     * reguly sa wyswietlane od 1. W celu zachowania spojnosci, od podanej
     * w argumencie wywolania liczby zostanie odjete 1 (w funkcji parse()):
     */
    int             rule_num;
};

/* Funkcja odpowiedzialna za zaalokowanie obszaru pamieci o rozmiarze 'size'. */
void *s_malloc(size_t size) {

    void* ptr;

    ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc() failed!\n");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/*
 * Argumentem funkcji jest adres wskaznika na obszar pamieci.
 * Funkcja zwalnia pamiec i ustawia wskaznik na NULL.
 */
void s_free(void** ptr) {

    if (*ptr) {
        free(*ptr);
        *ptr = NULL;
    }

}

/*
 * Argumentem funkcji jest adres wskaznika na strukture 'rule'.
 * Funkcja zwalnia pamiec i ustawia wskaznik na NULL.
 */
void free_rule(struct rule** ptr) {

    struct rule* r = *ptr;
    if (r) {
        s_free((void*)&r->chain);
        s_free((void*)&r->table);
        free(r);
        *ptr = NULL;
    }

}

/* Funkcja wypisuje informacje o bledzie i ustawia flage bledu: */
void parse_error(char* str, int* error) {
    fprintf(stderr, "%s", str);
    *error = 1;
}

/*
 * Funkcja przetwarza argumenty wywolania programu i w przypadku powodzenia
 * zwraca wskaznik na strukture 'rule'. Funkcja alokuje pamiec dla struktury
 * 'rule', nalezy pamietac o jej zwolnieniu.
 */
struct rule* parse(int argc, char ** argv) {

    int             error; /* Sygnalizuje wystapienie bledu. */
    struct rule     *r; /* Wskaznik na strukture przechowujaca regule. */
    void            *ptr;

    if (argc < 2) {
        fprintf(stderr, "parse(): invalid arguments\nTry '%s -h'\n",
                argv[0]);

        return NULL;
    }

    error = 0; /* Stan poczatkowy = brak bledu */

    /* Alokacja pamieci dla reguly: */
    r = (struct rule*)s_malloc(sizeof(struct rule));
    r->chain = NULL;
    r->table = NULL;
    r->operation = NONE;

    /* Parsowanie argumentow wywolania: */
    argv++;
    while (*argv) {

        if (!strcmp(*argv, "-t")) { /* Okreslenie nazwy tablicy. */

            argv++; /* Przejscie do argumentu po '-t'. */
            if ((*argv == NULL) || (**argv == '-')) {
                parse_error("Expecting <table> after -t\n",
                            &error);
                break;
            }

            /* Opcja '-t' wystepuje wiecej niz 1 raz: */
            if (r->table) {
                parse_error("Table can be specified "
                            "only once.\n", &error);
                break;
            }

            /*
             * Alokacja pamieci dla nazwy tablicy i
             * zapisanie nazwy w 'r->table'.
             * 'ptr' pozwala zachowac const dla 'r->table':
             */
            ptr = s_malloc(strlen(*argv) + 1);
            strcpy(ptr, *argv);
            r->table = ptr;
            ptr = NULL;

        } else if (!strcmp(*argv, "-N")) { /* Utworzenie lancucha */

            if (r->operation != NONE) {
                parse_error("Can't use -N with other option.\n",
                            &error);
                break;
            }

            argv++; /* Przejscie do argumentu po '-N'. */
            if ((*argv == NULL) || (**argv == '-')) {
                parse_error("Expecting <chain> after -N\n",
                            &error);
                break;
            }

            /*
             * Alokacja pamieci dla nazwy lancucha i
             * zapisanie nazwy w 'r->chain'.
             * 'ptr' pozwala zachowac const dla 'r->chain':
             */
            ptr = s_malloc(strlen(*argv) + 1);
            strcpy(ptr, *argv);
            r->chain = ptr;
            ptr = NULL;

            /* Regula NEW_CHAIN poprawna: */
            r->operation = NEW_CHAIN;

        } else if (!strcmp(*argv, "-D")) { /* Usuniecie reguly */

            if (r->operation != NONE) {
                parse_error("Can't use -D with other option.\n",
                            &error);
                break;
            }

            argv++; /* Przejscie do argumentu po '-D'. */
            if ((*argv == NULL) || (**argv == '-')) {
                parse_error("Expecting <chain> after -D\n",
                            &error);
                break;
            }

            /*
             * Alokacja pamieci dla nazwy lancucha i
             * zapisanie nazwy w 'r->chain'.
             * 'ptr' pozwala zachowac const dla 'r->chain':
             */
            ptr = s_malloc(strlen(*argv) + 1);
            strcpy(ptr, *argv);
            r->chain = ptr;
            ptr = NULL;

            /*
             * Przejscie do argumentu po '-D <chain>'
             * (numeru reguly do usuniecia):
             */
            argv++;
            if ((*argv == NULL) || (**argv == '-')) {
                parse_error("Expecting <rule number> after "
                            "-D <chain>\n", &error);
                break;
            }

            r->rule_num = atoi(*argv) - 1;
            if (r->rule_num < 0) {
                parse_error("Invalid <rule number>\n", &error);
                break;
            }

            /* Regula DELETE_RULE poprawna: */
            r->operation = DELETE_RULE;

        } else if (!strcmp(*argv, "-h")) { /* Pomoc. */

            fprintf(stdout,
                    "Options:\n"
                    "-D <chain> <rule num> Delete rule from chain\n"
                    "-N <chain> Create user-defined chain\n"
                    "-t <table> Table to manipulate\n");

            free_rule(&r);
            exit(EXIT_SUCCESS);

        } else {
            fprintf(stderr, "Unknown option: %s\n", *argv);
            error = 1;
            break;
        }

        argv++; /* Sprawdzamy kolejne argumenty wywolania. */
    }

    /* W przypadku braku '-D' lub '-N': */
    if ((!error) && (r->operation == NONE)) {
        fprintf(stderr, "No option specified!\n");
        error = 1;
    }

    /* W przypadku braku '-t': */
    if ((!error) && (r->table == NULL)) {
        fprintf(stderr, "No table specified!\n");
        error = 1;
    }

    if (error) {
        free_rule(&r); /* Ustawia 'r' na NULL. */
    }

    return r;
}


void delete_rule(struct xtc_handle *h, struct rule* r) {

    int retval;

    /* Sprawdzenie czy podany lancuch wystepuje w tablicy: */
    retval = iptc_is_chain(r->chain, h);
    if (!retval) {
        fprintf(stderr, "Chain '%s' does not exist in table '%s'!\n",
                r->chain, r->table);
        exit(EXIT_FAILURE);
    }

    /* Usuniecie reguly o okreslonym numerze: */
    retval = iptc_delete_num_entry(r->chain, (unsigned int) r->rule_num, h);
    if (!retval) {
        fprintf(stderr, "iptc_delete_num_entry(): %s\n",
                iptc_strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * Zatwierdzenie zmian. Funkcja zwalnia pamiec zaalokowana dla 'h' i
     * ustawia uchwyt na NULL:
     */
    retval = iptc_commit(h);
    if (!retval) {
        fprintf(stderr, "iptc_commit(): %s!\n",
                iptc_strerror(errno));
        exit(EXIT_FAILURE);
    }
}


void create_chain(struct xtc_handle *h, struct rule* r) {

    int retval;

    /* Utworzenie nowego lancucha: */
    retval = iptc_create_chain(r->chain, h);
    if (!retval) {
        fprintf(stderr, "iptc_delete_num_entry(): %s\n",
                iptc_strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * Zatwierdzenie zmian. Funkcja zwalnia pamiec zaalokowana dla 'h' i
     * ustawia uchwyt na NULL:
     */
    retval = iptc_commit(h);
    if (!retval) {
        fprintf(stderr, "iptc_commit(): %s!\n",
                iptc_strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {

    struct rule     *r; /* Wskaznik na regule. */
    struct xtc_handle   *h; /* Uchwyt. */

    /* parse() zwraca regule na podstawie argumentow wywolania programu: */
    r = parse(argc, argv);
    if (!r) {
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja dla tablicy okreslonej przez parametr 'r->table': */
    h = iptc_init(r->table);
    if (!h) {
        fprintf(stderr, "iptc_init(): %s\n", iptc_strerror(errno));
        exit(EXIT_FAILURE);
    }


    /* Usuniecie reguly: */
    if (r->operation == DELETE_RULE) {
        delete_rule(h, r);
    } else if (r->operation == NEW_CHAIN) { /* Utworzenie lancucha. */
        create_chain(h, r);
    }

    /* Zwolnienie pamieci zaalokowanej dla reguly w funkcji parse(): */
    free_rule(&r);

    if (h) {
        /* Zamkniecie uchwytu (funkcja nie powinna sie wywolac,
         * poniewaz iptc_commit() zwolnila uchwyt 'h'): */
        iptc_free(h);
    }

    exit(EXIT_SUCCESS);
}
