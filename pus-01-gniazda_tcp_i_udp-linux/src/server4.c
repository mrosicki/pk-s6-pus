/*
 * Celem zadania jest zaimplementowanie prostego serwera TCP, do którego
 *
 * można połączyć się za pomocą aplikacji telnet. Trudność polega na odpowiednim zor-
 * ganizowaniu operacji wejścia/wyjścia w taki sposób, aby serwer umożliwiał obsługę
 *
 * wielu klientów. Dane odebrane od jednego klienta (telnet) mają być wysyłane do po-
 * zostałych.
 *
 * Program serwera należy zaimplementować w oparciu o poniższe założenia:
 * 1. Serwer może odebrać dane z dowolnego interfejsu sieciowego (INADDR_ANY).
 *
 * 2. W celu organizacji operacji wejścia/wyjścia należy posłużyć się funkcją se-
 * lect().
 *
 * 3. Po odebraniu nowego połączenia TCP za pomocą funkcji accept(), serwer wy-
 * pisuje adres i numer portu klienta (można posłużyć się funkcją inet_ntop()).
 *
 * Deskryptor utworzony przez funkcję accept() należy dodać do zbioru deskryp-
 * torów monitorowanych przez funkcję select().
 *
 * 4. Zamknięcie połączenia przez aplikację telnet, ma powodować usunięcie odpo-
 * wiedniego deskryptora ze zbioru deskryptorów monitorowanych.
 *
 * 5. Dane tekstowe otrzymane od jednego klienta mają być przesyłane do wszyst-
 * kich pozostałych.
 */

#include <stdio.h>     // output
#include <stdlib.h>    // atoi, exit
#include <arpa/inet.h> // listen, bind, htons, ...
#include <unistd.h>    // close
#include <string.h>    // strlen, memset
#include <sys/time.h>  // FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE        1
#define FALSE       0
#define MAX_CLIENTS 5 // maksymalna ilość połączeń obsługiwanych przez serwer

const int MAX_BUF_SIZE = 256; // rozmiar tablic buforu
const int ARGS_SIZE    = 2;   // name, port

int listenfd;                    // deskryptor nasluchujacy
int clients_connfd[MAX_CLIENTS]; // lista deskryptorów podłączonych klientów

fd_set readset; //  zbiór adresów podłączonych klientów
int maxfd;      // maksymalny deskryptor ze wszystich podłączonych

// obsługa błędu, błąd identyfikowany jest gdy result < 0
void handle_error(int result, const char * message, int exit_with_error) {
    if (result < 0) {
        perror(message);

        if (exit_with_error == 1) { // parametr wskazuje czy wykrycie błędu wykrycie błędu powinno wyłączyć serwer
            close(listenfd);
            exit(EXIT_FAILURE);
        }
    }
}

int get_number_connected_clients() { //  zliczenie ilosci aktualnie podłączonych klientów
    int connected = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_connfd[i] > 0) {
            connected++;
        }
    }

    return connected;
}

// wysłanie wiadomości do klienta poprzez deskryptor
void send_message(int connfd, const char * message) {
    int result = send(connfd, message, strlen(message), 0);

    handle_error(result, "send()", FALSE);
}

// wysłanie wiadomości do wszystkich klientów oprócz jednego
void send_message_to_other(int excludedfd, const char * message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_connfd[i] > 0 && clients_connfd[i] != excludedfd) {
            send_message(clients_connfd[i], message);
        }
    }
}

void add_client_connection(int clientfd) {
    char buf[MAX_BUF_SIZE]; // bufor wiadomości

    // sprawdzenie czy klient może być podłączony
    if (get_number_connected_clients() < MAX_CLIENTS) {
        for (int i = 0; i < MAX_CLIENTS; i++) { // dodanie klienta do listy z deskryptorami
            if (clients_connfd[i] <= 0) {
                clients_connfd[i] = clientfd;
                break;
            }
        }

        memset(buf, 0, sizeof(buf)); // wyczyszczenie obszaru pamięci buforu
        sprintf(buf, "Nawiazano polaczenie. Twoje id: %d\n", clientfd);
        send_message(clientfd, buf);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "Do pokoju dolaczyl klient id: %d\n", clientfd);
        send_message_to_other(clientfd, buf);
    } else {
        send_message(clientfd, "Polaczenie odrzucone, zbyt duze obciazenie serwera.\n");
        close(clientfd); // zamknięcie połączenia z odrzuconym klientem
    }
}

// wykonanie akcji na zdarzenie od serwera (podłączenie klienta)
void handle_server_action() {
    struct sockaddr_in client_addr;                                                     // struktura na dane klienta
    socklen_t addr_len = sizeof(client_addr);                                           // rozmiar struktury
    int new_clientfd   = accept(listenfd, (struct sockaddr *) &client_addr, &addr_len); // odebranie deskryptora podłączonego klienta

    handle_error(new_clientfd, "accept()", TRUE);

    if (new_clientfd > 0) {
        char buf[MAX_BUF_SIZE];
        memset(buf, 0, sizeof(buf));

        inet_ntop(AF_INET, (void *) &client_addr, buf, sizeof(buf)); // konwersja na czytelny adres ip
        int client_port = ntohs(client_addr.sin_port);               // konwersja na czytelny numer portu
        fprintf(stderr, "Polaczenie zaakceptowane od %s:%d\n", buf, client_port);
        add_client_connection(new_clientfd);
    }
}

// wykonanie metody gdy wykryto odłączenie klienta
void disconect_client(int clientfd) {
    for (int i = 0; i < MAX_CLIENTS; i++) { // wyszukanie na liscie klientow deskryptora odlaczonego klienta
        if (clients_connfd[i] == clientfd) {
            clients_connfd[i] = 0;                                         // wyzerowanie deskryptora
            send_message_to_other(clientfd, "Jeden z gosci opuscil czat"); // komunikat dla innych
            fprintf(stderr, "Klient id: %d opuscil czat\n", clientfd);     // komunikat dla serwera
            break;
        }
    }
}

// wykonanie gdy wykryto zdarzenie od klienta (wiadomosc lub odlaczenie)
void handle_client_action() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int cfd = clients_connfd[i];
        if (cfd > 0 && FD_ISSET(cfd, &readset)) { // wykrycie od którego klienta wyszło zdarzenie
            char buf[MAX_BUF_SIZE];               // bufor dla wiadomości
            memset(buf, 0, strlen(buf));
            int result = recv(cfd, buf, MAX_BUF_SIZE, 0); // odebranie wiadomości, parametr 0 - bez flag
            handle_error(result, "recv()", FALSE);
            char message_to_send[MAX_BUF_SIZE]; // zformatowana wiadomości dla klientów i serwera
            memset(message_to_send, 0, sizeof(message_to_send));
            sprintf(message_to_send, "[%d]: %s", cfd, buf);

            if (result == 0) { // wartość 0 oznacza odłączenie klienta - wysyła znak końca pliku
                disconect_client(cfd);
            } else if (result > 0) { // jeżeli wiadomość została odebrana poprawnie recv zwróci jej rozmiar
                send_message_to_other(cfd, message_to_send);
                fprintf(stderr, "%s", message_to_send);
            }
        }
    }
}

// aktualizacja readset po każdym wystąpieniu zdarzenia
void update_readset() {
    FD_ZERO(&readset); // zerowanie zbioru
    maxfd = listenfd;  // inicjalizacja deskryptorem serwera
    FD_SET(listenfd, &readset);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients_connfd[i] > 0) {
            FD_SET(clients_connfd[i], &readset); // dodanie do zbioru deskryptora podłączonego klienta
            if (clients_connfd[i] > maxfd) {
                maxfd = clients_connfd[i]; // jeżeli deskryptor jest większy niż max, przypisanie do max
            }
        }
    }
}

int main(int argc, char const * argv[]) {
    int result; // zmienna na wzracane przez metody sieciowe wartości

    if (argc != ARGS_SIZE) { // sprawdzenie czy ilość argumentów się zgadza
        puts("Args error: <file> <port>");
        exit(EXIT_FAILURE);
    }

    const int PORT = atoi(argv[1]);

    listenfd = socket(PF_INET, SOCK_STREAM, 0); // utworzenie gniazda dla serwera
    handle_error(listenfd, "socket()", TRUE);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // zerowanie struktury adresu serwera
    for (size_t i = 0; i < MAX_CLIENTS; i++) {    // zerowanie listy klientów
        clients_connfd[i] = 0;
    }

    server_addr.sin_family      = AF_INET;           // zawsze na AF_INET
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // umożliwienie połączenia dowolnego interfejsu
    server_addr.sin_port        = htons(PORT);       // ustawienie portu serwera na ten z listy argumentów

    result = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)); // przypisanie adresu do utworzonego socketa
    handle_error(result, "bind()", TRUE);

    result = listen(listenfd, 2); // ustawienie gniazda jako pasywne, nasłuchujące, może teraz akceptować połączenia
    handle_error(result, "listen()", TRUE);

    puts("Oczekiwanie na klientow");

    while (1) {
        update_readset();                                       // aktualizacja zbioru deskryptorów
        result = select(maxfd + 1, &readset, NULL, NULL, NULL); // monitorowanie wielu deskryptorów i oczekiwanie, aż jeden z nich będzie gotowy
        handle_error(result, "select()", TRUE);
        if (FD_ISSET(listenfd, &readset)) { // sprawdzenie czy zdarzenie serwera czy klienta
            handle_server_action();
        } else {
            handle_client_action();
        }
    }

    close(listenfd);

    return 0;
} /* main */
