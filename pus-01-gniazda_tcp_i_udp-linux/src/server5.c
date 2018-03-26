#include <stdio.h>     // output
#include <stdlib.h>    // atoi, exit
#include <arpa/inet.h> // listen, bind, htons, ...
#include <unistd.h>    // close
#include <string.h>    // strlen, memset
#include <sys/time.h>  // FD_SET, FD_ISSET, FD_ZERO macros
#include <ctype.h>     // tolower;
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#define GRAPHIC_EXT_SIZE 4
#define BUFF_SIZE        1024
#define IMG_DIR          "img"
#define IMG_DIR_TEST     "../bin/img"


#define HTML_TAG   "<html>%s</html>"
#define HEAD_TAG   "<head>%s</head>"
#define BODY_TAG   "<body>%s</body>"
#define IMG_TAG    "<img src=\"%s\" />"
#define CENTER_TAG "<center>%s</center>"


#define HTTP_OK                 "HTTP/1.1 200 OK\n"
#define HTTP_CONTENT_TYPE_HTML  "Content-Type: text/html\n"
#define HTTP_ACCEPT_RANGES_BYTE "accept-ranges: bytes\n"
#define HTTP_CONTENT_TYPE_IMG   "Content-Type: image/%s\n"
#define HTTP_CONTENT_LEN        "Content-Length: %d\n"
#define HTTP_CONNECTION_CLOSE   "Connection: close\n"
#define HTTP_CONTENT_HTML       "\n%s"
#define HTTP_CONTENT_IMG        "\n%s"

char * GRAPHIC_EXT[4] = { "jpg", "jpge", "png", "gif" };


#define TRUE  1
#define FALSE 0

typedef int bool;

int listenfd; // deskryptor nasluchujacy

typedef struct { // struktura do przenoszenia danych do watku
    int connfd;
} conn_data;

typedef struct { // struktura do przenoszenia nazw plikow odczytanych z folderu
    char ** content;
    int     size;
} dir_content_t;

bool is_graphic_ext(char * ext) { // sprawdzenie po rozszrzeniu czy plik jest grafika
    if (strlen(ext) > 0) {
        for (size_t i = 0; i < GRAPHIC_EXT_SIZE; i++) {
            if (strcmp(GRAPHIC_EXT[i], ext) == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void reverse_array(char * array) { // odwracanie tablicy c stringu
    int len = strlen(array);
    int i = 0, j = len - 1;

    while (i < j) {
        array[i] ^= array[j];
        array[j] ^= array[i];
        array[i] ^= array[j];
        i++;
        j--;
    }
}

char * get_extension(char * file_uri) { // czytanie rozszrzenia ze sciezki do pliku
    int file_uri_index = strlen(file_uri) - 1;
    char * buff        = malloc(strlen(file_uri));
    int buff_index     = 0;

    // czytanie odbywa sie od tylu i konczy gdy napotka kropke
    while (file_uri_index >= 0 && file_uri[file_uri_index] != '.') {
        buff[buff_index] = tolower(file_uri[file_uri_index--]);
        buff_index++;
    }
    buff[buff_index] = 0;
    reverse_array(buff); // odczytane znaki sa od tylu, trzeba je odwrocic
    return buff;
}

// pobiera strukture z lista wszystkich nazw plikow i wybiera tylko te graficzne
dir_content_t * filter_graphic_only(dir_content_t * dir_content) {
    int index = 0;
    char ** img_name_only_content = malloc(sizeof(char *)); // rezerwacja miejsca na wskaznik do cstringu
    dir_content_t * img_name_only = malloc(sizeof(dir_content_t));

    for (int i = 0; i < dir_content->size; i++) {
        if (is_graphic_ext(get_extension(dir_content->content[i]))) {
            img_name_only_content[index] = malloc(strlen(dir_content->content[i])); // rezerwacja miejsca na nazwe
            strcpy(img_name_only_content[index], dir_content->content[i]);
            // za kazdym razem dynamicznie rozszerzeamy tablice dodajac jeden dodatkowy wskaznik na nowa nazwe
            img_name_only_content = realloc(img_name_only_content, ((++index) + 1) * sizeof(char *));
        }
    }
    img_name_only->content = img_name_only_content;
    img_name_only->size    = index;
    return img_name_only;
}

// metoda zwraca nazwy wszystkich plikow w podanym folderze
dir_content_t * get_dir_content(char * dirpath) {
    struct dirent * entry;                                     // struktura w ktorej bedzie informacja o jednym pliku
    char ** content = malloc(sizeof(char *));                  // wszystkie nazwy plikow
    int index       = 0;                                       // ilosc nazw plikow
    dir_content_t * dir_content = malloc(sizeof(dir_content)); // struktura do przekazania danych poza metode

    DIR * dir = opendir(dirpath);

    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) { // readddir za kazdym wywolaniem zwraca kolejna strukture entry
            content[index] = malloc(strlen(entry->d_name));
            strcpy(content[index], entry->d_name); // przekopiowanie nazwy pliku do naszej tablicy
            index++;
            // za kazdym razem dynamicznie rozszerzeamy tablice dodajac jeden dodatkowy wskaznik na nowa nazwe
            content = realloc(content, sizeof(char *) * (index + 1));
        }
    }
    dir_content->content = content;
    dir_content->size    = index;

    return dir_content;
}

// łaczenie sciezki do folderu i nazwy pliku
char * build_path(char * dir_path, char * filename) {
    int buff_size = strlen(dir_path) + strlen(filename) + 5; // 5 zeby nie braklko miejsca
    char * buff   = malloc(buff_size);

    buff = memset(buff, 0, buff_size);
    strcat(buff, dir_path);
    strcat(buff, "/");
    strcat(buff, filename);
    return buff;
}

// tworzenie nowego tagu i dodawaniem do niego tresci
char * build_tag(char * tag, char * content) {
    int buff_size = strlen(tag) + strlen(content) + 10;
    char * buff   = malloc(buff_size);

    memset(buff, 0, buff_size);

    sprintf(buff, tag, content);
    return buff;
}

// tworzenie wnetrza tagu gdy mamy do dodania kilka tagow
char * build_tag_content(char ** tags, size_t size) {
    char * buff = malloc(10);

    buff = memset(buff, 0, 10);
    if (size > 0) {
        for (int i = 0; i < size; i++) {
            buff = realloc(buff, strlen(buff) + strlen(tags[i]) + 10);
            strcat(buff, tags[i]);
            strcat(buff, "\n");
        }
    }
    return buff;
}

// ladowanie pliku ze sciezki do struktury
FILE * load_image(char * file_uri) {
    FILE * image = fopen(file_uri, "r");

    if (image == NULL) {
        fprintf(stderr, "Blad podczas czytania obrazu: %s\n", file_uri);
    }
    return image;
}

// tworzenie naglowka dla odpowiedzi html
char * build_html_response(char * content) {
    // szczegoly na gorze w define
    char * header_format =
      HTTP_OK
      HTTP_CONTENT_TYPE_HTML
      HTTP_CONTENT_LEN
      HTTP_CONNECTION_CLOSE
      HTTP_CONTENT_HTML
    ;
    // alokowanei miejsca na naglowek, liczbe oraz tresc wiadomosci
    int c_len   = strlen(content);
    char * buff = malloc(strlen(header_format) + 20 + c_len);

    sprintf(buff, header_format, c_len, content);

    return buff;
}

// parsowanie naglowka http i wybieranie sciezki do pliku
// GET  /dir/file.gif /
char * get_file_uri_from_request(char * request) {
    char * file_uri       = malloc(BUFF_SIZE);
    size_t request_size   = strlen(request);
    size_t request_index  = 0; // index do przesuwania sie po cstringu zapytania
    size_t file_uri_index = 0; // index do przesuwania sie w tablicy ze sciezka do pliku
    size_t space_counter  = 0; // liczenie spacji w naglowku (taka metoda rozroznienia pliu na podstawie wygladu naglowka)
    int count_slash       = 0; // liczenie slashy w naglowku (taka metoda rozroznienia pliu na podstawie wygladu naglowka)

    // jesli wystapi druga spacja wiadomo, że sciezka do pliku juz nie wystapi
    while (request_index < request_size && space_counter < 2) {
        if (request[request_index] == ' ') {
            space_counter++;
        } else if (request[request_index] == '/' && space_counter == 1 &&
          count_slash == 0)
        {
            count_slash++;
        } else if (space_counter == 1) {
            file_uri[file_uri_index] = request[request_index];
            file_uri_index++;
        }
        request_index++;
    }
    file_uri[file_uri_index] = 0;
    return file_uri;
}

// sprawdzenie czy zapytanie dotyczy pliku graficznego
bool is_image_request(char * request) {
    char * file_uri = get_file_uri_from_request(request);
    char * file_ext = get_extension(file_uri);

    return is_graphic_ext(file_ext);
}

// wrapper na odbieranie
bool Recv(int __fd, void * __buf, size_t __n, int __flags) {
    if (recv(__fd, __buf, __n, __flags) < 0) {
        fprintf(stderr, "Blad podczas czytania, watek: %d\n", getpid());
        close(__fd);
        return FALSE;
    }
    return TRUE;
}

// wrapper na wysylanie
bool Send(int __fd, void * __buf, size_t __n, int __flags) {
    if (send(__fd, __buf, __n, __flags) < 0) {
        fprintf(stderr, "Blad podczas wysylania wiadomosci, deskryptor: %d", __fd);
        return FALSE;
    }

    fprintf(stderr, "Wyslano odpowiedz na zapytanie, deskryptor: %d\n", __fd);
    return TRUE;
}

// obsługa zapytania html
void handle_html_request(int * connfd) {
    dir_content_t * dir_content = get_dir_content(IMG_DIR);         // wszystkie nazwy plikow w folderze img
    dir_content_t * img_only    = filter_graphic_only(dir_content); // tylko nazwy plikow graficznych

    free(dir_content);

    char ** img_tags     = malloc(sizeof(char *) * img_only->size); // rezerwacj miejsca na wskazniki do cstringow
    size_t img_tags_size = img_only->size;
    for (int i = 0; i < img_only->size; i++) {
        char * filename     = img_only->content[i];
        char * path_to_file = build_path(IMG_DIR, filename);
        free(filename);
        img_tags[i] = build_tag(IMG_TAG, path_to_file); // dodanie cstrigu do listy tagow
    }

    // kolejno budowanie tresci
    char * center_content = build_tag_content(img_tags, img_tags_size);
    char * center_tag     = build_tag(CENTER_TAG, center_content);
    char * body_tag       = build_tag(BODY_TAG, center_tag);
    char * html_tag       = build_tag(HTML_TAG, body_tag);
    char * response       = build_html_response(html_tag);

    Send(*connfd, response, strlen(response), 0);
}

// budowanie naglowka dla odpowiedzi ze zdjeciem
char * build_img_response_header(FILE * image_file, char * filetype) {
    size_t img_size = 0;

    if (image_file != NULL) {
        fseek(image_file, 0, SEEK_END); // przejscie na koniec pliku
        img_size = ftell(image_file);   // sprawdzenie pozycji = rozmiar
        fseek(image_file, 0, SEEK_SET); // powrot na poczatek pliku
    }

    // linijki naglowka, szczegoly przy define
    char header_format[] =
      HTTP_OK
      HTTP_ACCEPT_RANGES_BYTE
      HTTP_CONTENT_TYPE_IMG
      HTTP_CONTENT_LEN
      HTTP_CONTENT_IMG
    ;

    char * buff = malloc(strlen(header_format) + 20);
    // pisanie pustej tresci bo reszta bedzie dosylana w kolejnych wiadoscia za naglowkiem
    sprintf(buff, header_format, filetype, img_size, "");

    return buff;
}

// obsluga zapytania typu zdjecie
void handle_img_request(int * connfd, char * buff) {
    char * file_uri   = get_file_uri_from_request(buff);                 // czytanie sciezki do pliku
    char * filetype   = get_extension(file_uri);                         // czytanie rozszerzenia
    FILE * image_file = load_image(file_uri);                            // ladowanie zdjecia
    char * header     = build_img_response_header(image_file, filetype); // tworzenie naglowka http

    Send(*connfd, header, strlen(header), 0);
    if (image_file != NULL) {
        int img_buffer_size = 10240; // bufor na czytanie zdjecia ograiczony do tego rozmiaru
        char img_buffer[img_buffer_size];
        while (!feof(image_file)) {                                               // jesli koniec koniec
            size_t read_size = fread(img_buffer, 1, img_buffer_size, image_file); // cztanie do buforu
            send(*connfd, img_buffer, read_size, 0);                              // wysylanie czesci do klienta
        }
    }
}

// metoda uruchamiana przez nowy watek, zajmuje sie obsluga nowego polaczenia
void * handle_connection(void * arg) {
    int * connfd = (int *) arg; // deskryptor polaczenia
    char * buff  = malloc(BUFF_SIZE);

    memset(buff, 0, BUFF_SIZE);

    fprintf(stderr, "Oczekiwanie na zapytanie\n");
    Recv(*connfd, buff, BUFF_SIZE, 0); // odebranie zapytania http
    if (is_image_request(buff)) {      /// sprawdzeni czy chodzi o zdjecie
        handle_img_request(connfd, buff);
    } else {
        handle_html_request(connfd); // czy o dokument
    }

    fprintf(stderr, "Polaczenie obsluzone\n");
    close(*connfd); // wszystko gotowe, koniec polaczenia
    free(arg);
    pthread_exit(NULL);
} /* handle_connection */

// obsługa błędu, błąd identyfikowany jest gdy result < 0
void handle_error(int result, char * message, int exit_with_error) {
    if (result < 0) {
        perror(message);

        if (exit_with_error == 1) { // parametr wskazuje czy wykrycie błędu wykrycie błędu powinno wyłączyć serwer
            close(listenfd);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char * argv[]) {
    int result;

    if (argc != 2) { // nazwa, port
        fprintf(stderr, "<program> <port>\n");
        exit(EXIT_FAILURE);
    }

    int PORT = atoi(argv[1]);

    listenfd = socket(PF_INET, SOCK_STREAM, 0); // utworzenie gniazda dla serwera
    handle_error(listenfd, "socket()", TRUE);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // zerowanie struktury adresu serwera

    server_addr.sin_family      = AF_INET;           // zawsze na AF_INET
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // umożliwienie połączenia dowolnego interfejsu
    server_addr.sin_port        = htons(PORT);       // ustawienie portu serwera na ten z listy argumentów

    result = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)); // przypisanie adresu do utworzonego socketa
    handle_error(result, "bind()", TRUE);

    result = listen(listenfd, 2); // ustawienie gniazda jako pasywne, nasłuchujące, może teraz akceptować połączenia
    handle_error(result, "listen()", TRUE);

    puts("Uruchomienie serwera. Oczekiwanie na polaczanie\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int clientfd;
        pthread_t tid;
        if ((clientfd = accept(listenfd, (struct sockaddr *) &client_addr, &addr_len)) > 0) {
            fprintf(stderr, "Wykryto probe polaczenia, deskryptor: %d\n", clientfd);
            conn_data * data = malloc(sizeof(conn_data));
            data->connfd = clientfd;
            pthread_create(&tid, NULL, handle_connection, data);
        }
        pthread_join(tid, NULL);
    }

    return 0;
} /* main */
