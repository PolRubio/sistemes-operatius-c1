#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>


#define MAX_LINE 100

int main(int argc, char *argv[]){       
    if(argc>3){
        printf("Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        return 0;
    }
    int PORT=(argc>=2)?atoi(argv[1]):8080;
    char *IP=(argc>2)?argv[2]:"127.0.0.1";
    int sock_fd;

    char send_txt[MAX_LINE];
    char recv_txt[MAX_LINE];

    struct sockaddr_in servaddr;

    printf("Client side\n");
    printf("Connecting to %s:%d\n", IP, PORT);

    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0){
        perror("socket");
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);

    if(inet_pton(AF_INET, IP, &(servaddr.sin_addr))!= 1) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return 1;
    }

    int min=0, max=100, num = max/2, result, totaliterations=0;


    do{
        bzero(recv_txt, MAX_LINE);
        bzero(send_txt, MAX_LINE);

        printf("Numero intentado: %d\n", num);
        
        sprintf(send_txt, "%d", num);
        
        if (write(sock_fd, send_txt, strlen(send_txt)) < 0) {
            perror("write");
            return 1;
        }

        if (read(sock_fd, recv_txt, MAX_LINE) < 0) {
            perror("read");
            return 1;
        }
        printf("Respuesta servidor: %s\n",recv_txt);

        result = atoi(recv_txt);

        if(max-min==1){
            printf("Random number is the max\n");
            num = max;
        }
        else if(result==0){
            printf("\tRandom number is equal to %d\n", num);
            break;
        }
        else if(result==1){
            printf("\tRandom number is less than %d\n", num);
            max = num;
            num = (max+min)/2;
        }
        else{
            printf("\tRandom number is greater than %d\n", num);
            min = num;
            num = (max+min)/2;
        }
        totaliterations++;
        printf("\n\n");
    }while(result!=0);
    printf("Total iterations: %d\n", totaliterations);
    return(0);
}
