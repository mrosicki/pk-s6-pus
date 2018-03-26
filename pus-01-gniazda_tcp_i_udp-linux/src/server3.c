#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>
#include "libpalindrome.h"




int main(int argc, char** argv) {

    int             sockfd; /* Deskryptor gniazda. */
    int             retval; /* Wartosc zwracana przez funkcje. */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez recvfrom() i sendto(): */
    char            buff[256];

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];

    
    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    
    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr     =       htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port            =       htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);

    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(sockfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is listening for incoming connection...\n");

    client_addr_len = sizeof(client_addr);

    /* Oczekiwanie na dane od klienta: */
    int flag_empty = 0;
    int flag_not_number =0;
    int i;
    //petla dzialajaca dopoki nie odbierzemy pustej ramki, flaga w sumie moze zostac zamieniona na while(1)
    while(1){

    //tutaj przekopiowane z poprzedniego cwiczenia
    retval = recvfrom(
                 sockfd,
                 buff, sizeof(buff),
                 0,
                 (struct sockaddr*)&client_addr, &client_addr_len
             );
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }
    //printf do pomocy w debugowaniu
    //printf("%d : %d",client_addr.sin_port,ntohs(client_addr.sin_port));

    //zamiana adresu z postaci binarnej na text i zapisanie go w addr_buff 
    inet_ntop(AF_INET, (struct sockaddr*)&client_addr.sin_addr,addr_buff,sizeof(addr_buff));
    fprintf(stdout,"Odebrano wiadmosc od: %s:%d\n",addr_buff,ntohs(client_addr.sin_port));
    
    //sprawdzenie czy odebrana wiadomosc jest pusta, flaga jest zbedna przy instrukcji break
    if(buff[0]==NULL){
       // printf("flaga zmieniona\n");
        break;
    }
    /*sprawdzenie czy wiadomosc nadaje sie do sprawdzenia czy jest palindromem 
    tzn. czy sklada sie z samych cyfr i jest bez spacji */
    for(i=0;i<sizeof(buff) && buff[i]!= '\n';i++){
        if((buff[i]>='0' && buff[i]<='9')){
            flag_not_number = 0;
        }else{
            flag_not_number = 1;
            break;
        }
        if(buff[i]==' '){
            flag_not_number = 1;
            break;
        }
    
    }
    //bufor dla wiadomosci zwrotnej ponizej
    char mess[256];
    //funkcja is_palindrome jest uruchamiana jesli flaga nie zostala ustawiona przez petle powyzej
    if(flag_not_number==0){
        printf("Sprawdzam ciag %s\n",buff);
        
        if(is_palindrome(buff,sizeof(buff))){
            printf("IS_palindrome %d, sizeof %ld\n",is_palindrome(buff,sizeof(buff)),sizeof(buff));
            strcpy(mess,"Ciag jest palindromem\n");
        }else{
            strcpy(mess,"Ciag nie jest palindromem\n");
            printf("IS_palindrome %d\n",is_palindrome(buff,sizeof(buff)));
        }
    }else{
        strcpy(mess,"Ciag nie sklada sie z samych cyfr\n");
    }
        //wyslanie wiadomosci do clienta
        sendto(sockfd,mess,sizeof(mess),0,(struct sockaddr*)&client_addr,client_addr_len);

        memset(&mess, 0, sizeof(mess));
    }
    printf("Zatrzymanie serwera\n");
    close(sockfd);

    return 0;
}