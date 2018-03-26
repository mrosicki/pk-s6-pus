#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>

int main(int argc, char** argv){

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t       addr_len;               /* Rozmiar struktury w bajtach. */
    char            buff[256];              /* Bufor dla funkcji recvfrom(). */





    if (argc != 3) {
        fprintf(
            stderr,
            "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]
        );
        exit(EXIT_FAILURE);
    }

      /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    
    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    remote_addr.sin_family = AF_INET;
    //zerowanie buffora do wsylania na wszelki wypadek
    memset(&buff, 0, sizeof(buff));
    remote_addr.sin_port = htons(atoi(argv[2])); /* Numer portu. */
    
    addr_len = sizeof(remote_addr); /* Rozmiar struktury adresowej w bajtach. */
    connect(sockfd, (struct sockaddr*)&remote_addr, addr_len);
    
    int i;
    //jedynym warunkiem wyjscia z petli jest wyslanie pustej ramki, co osiagamy instrukcja break ponizej
    while(1){
    fgets(buff,sizeof(buff),stdin);
    /*(char)10 to znak '\n' ktory i tak zostaje wyslany. Zeby po wcisnieciu entera poszla pusta ramka
    to trzeba zamienic ten znak na znak konca tablicy co daje pusty bufor*/
    if(buff[0]==(char)10){
        buff[0]='\0';
        send(sockfd,buff,sizeof(buff),0);
        
        break;
    }
    //wypisanie zawartosci bufora do pomocy w ewentualnym debugowaniu
    printf("BUFOR %s\n",buff);
    /*
    for(i=0;i<sizeof(buff);i++){
        printf("%d",(int)buff[i]);
    }
*/
    //wyslanie do adresu pod ktory zostalo podpiete gniazdo za pomoca connect
    send(sockfd,buff,sizeof(buff),0);
    
    //odbieranie wiadomosci zwrotnej i ladowanie jej do bufora
    retval = recv(sockfd,buff,sizeof(buff),0);
     
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }
    //ustawienie znaku konca tablicy na koncu wiadomosci zwrotnej zaladowanej do bufora
    buff[retval]='\0';
    // wypisanie zawartosci bufora z wiadomoscia zwrotna
    printf("Odpowiedz serwera: %s",buff);
     memset(&buff, 0, sizeof(buff));

    }
    //zamkniecie gniazda i wyjscie z programu
    close(sockfd);
    exit(EXIT_SUCCESS);



 return 0;
}