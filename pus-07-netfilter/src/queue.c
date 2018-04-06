/*
 * Data:                2009-04-15
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc queue.c -o queue -lnetfilter_queue
 * Uruchamianie:        $ ./queue
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h> /* if_indextoname() */
#include <linux/types.h>
#include <linux/netfilter.h> /* NF_ACCEPT */
#include <limits.h> /* Wymagane przez <linux/netfilter_ipv4.h> (MIN_INT, ...) */
#include <linux/netfilter_ipv4.h> /* Nazwy punktow zaczepienia Netfilter. */
#include <libnetfilter_queue/libnetfilter_queue.h>

static int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                    struct nfq_data *nfa, void *data);

static u_int32_t print_packet(struct nfq_data *tb);

int main(int argc, char **argv) {

    struct nfq_handle       *h; /* Uchwyt polaczenia. */
    struct nfq_q_handle     *qh; /* Uchwyt kolejki. */
    int                     fd; /* Deskryptor gniazda Netlink. */
    int                     rv; /* Wartosc zwracana przez funkcje. */
    int                     queue_num; /* Numer kolejki. */
    /*
     * Bufor dla pakietu. Specyfikacja trybutu gcc - aligned.
     * Wartosc atrybutu nie zostala okreslona = kompilator jest
     * odpowiedzialny za wyrownanie typu.
     */
    char buf[4096] __attribute__((aligned));


    /* Pobranie uchwytu polaczenia z podsystemem jadra odpowiedzialnym
     * za kolejkowanie pakietow: */
    h = nfq_open();
    if (!h) {
        fprintf(stderr, "nfq_open() failed!\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Dobra praktyka jest wywolanie najpierw nfq_unbind_pf() i pozniej
     * nfq_bind_pf() w celu okreslenia rodziny protokolow.
     */
    if (nfq_unbind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "nfq_unbind_pf() failed!\n");
        exit(EXIT_FAILURE);
    }

    /* Powiązanie z rodziną AF_INET: */
    if (nfq_bind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "nfq_bind_pf() failed!\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Powiazanie gniazda z kolejka o numerze 5 i
     * okreslenie funkcji callback:
     */
    queue_num = 5;
    qh = nfq_create_queue(h,  queue_num, &callback, NULL);
    if (!qh) {
        fprintf(stderr, "nfq_create_queue() failed!\n");
        exit(EXIT_FAILURE);
    }

    /* Tryb kopiowania (pakiet + meta-dane) i rozmiar pakietu: */
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        fprintf(stderr, "nfq_set_mode() failed!\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Using queue %d and mode NFQNL_COPY_PACKET.\n"
            "Waiting for packets...\n", queue_num);

    /* Pobranie deskryptora gniazda: */
    fd = nfq_fd(h);

    /* Obieranie pakietow: */
    while ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {
        fprintf(stdout, "Packet received. Entering callback.\n");
        /* Wywolanie funkcji callback i przekazanie jej pakietu: */
        nfq_handle_packet(h, buf, rv);

        sleep(1); /* Celowe opoznienie o 1s. */
    }

    /* Zwolnienie uchwytu kolejki: */
    nfq_destroy_queue(qh);

    /* Zamkniecie polaczenia i zwolnienie zwiazanych z nim zasobow: */
    nfq_close(h);

    exit(EXIT_SUCCESS);
}

/*
 * Parametr 'data' nie zawiera pakietu, a dane uzytkownika przekazane do funkcji
 * callback za pomoca nfq_create_queue(). W naszym przypadku NULL.
 */
static int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                    struct nfq_data *nfa, void *data) {

    u_int32_t id; /* ID pakietu. */

    /* Wypisanie informacji na temat pakietu: */
    id = print_packet(nfa);

    fprintf(stdout, "Leaving callback.\n\n");

    /*
     * Okreslenie werdyktu.
     * Pakiet nie jest modyfikowany (nie zachodzi koniecznosc podania
     * bufora, w ktorym znajduje sie zmodyfikowany pakiet):
     */
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}


static u_int32_t print_packet(struct nfq_data *tb) {

    int                             id = 0; /* ID pakietu. */
    struct nfqnl_msg_packet_hdr     *ph; /* Naglowek meta-danych. */
    struct nfqnl_msg_packet_hw      *hwph; /* Adres warstwy lacza anych. */
    u_int32_t                       ifi; /* Indeks interfejsu. */
    int                             retval; /* Wartosc zwracana. */
    char                            *data; /* Bufor na payload. */
    char                            *hook; /* Punkt zaczepienia Netf. */
    char                            ifname[IF_NAMESIZE]; /* Nazwa interf. */

    /* Naglowek z meta-danymi: */
    ph = nfq_get_msg_packet_hdr(tb);
    if (ph) {
        /* ID pakietu w porzadku sieciowym: */
        id = ntohl(ph->packet_id);
        /* Wartosc pola EtherType/Type ramki Ethernet: */
        fprintf(stdout, "ID: '%u', EtherType: '0x%04x', ",
                id, ntohs(ph->hw_protocol));

        fprintf(stdout, "Netfilter Hook: ");

        switch (ph->hook) {
        case NF_IP_PRE_ROUTING:
            hook = "PREROUTING";
            break;
        case NF_IP_LOCAL_IN:
            hook = "INPUT";
            break;
        case NF_IP_FORWARD:
            hook = "FORWARD";
            break;
        case NF_IP_LOCAL_OUT:
            hook = "OUTPUT";
            break;
        case NF_IP_POST_ROUTING:
            hook = "POSTROUTING";
            break;
        default:
            hook = "UNKNOWN";
        }

        fprintf(stdout, "'%s',\n", hook);

    }

    /* Pobranie adresu warstwy lacza danych (zrodlowego): */
    hwph = nfq_get_packet_hw(tb);
    if (hwph) {
        int i, hlen = ntohs(hwph->hw_addrlen);

        fprintf(stdout, "Source Address: '");
        for (i = 0; i < hlen-1; i++)
            fprintf(stdout, "%02x:", hwph->hw_addr[i]);
        fprintf(stdout, "%02x', ", hwph->hw_addr[hlen-1]);
    }


    /* Pobranie indeksu interfejsu wejsciowego: */
    ifi = nfq_get_indev(tb);
    if (ifi) {
        /* Konwersja indeksu na nazwe interfejsu: */
        if (if_indextoname(ifi, ifname) == NULL) {
            perror("if_indextoname()");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Input Dev: '%s', ", ifname);
    }

    /*
     * Pobranie indeksu interfejsu wyjsciowego
     * (na danym etapie moze nie byc znany):
     */
    ifi = nfq_get_outdev(tb);
    if (ifi) {

        if (if_indextoname(ifi, ifname) == NULL) {
            perror("if_indextoname()");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Output Dev: '%s', ", ifname);
    }


    retval = nfq_get_payload(tb, &data);
    if (retval >= 0) {
        fprintf(stdout, "\nPayload Length: '%d'\n", retval);
    }

    return id;
}
