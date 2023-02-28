#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>


#define MAX_LINE 100

int compared(int randnum, int num){
  if(num>randnum) return 1;
  else if(num<randnum) return -1;
  else return 0;
}

int random_number_gen(int min_range, int max_range, int seed){
    time_t t;
    long long int current_time = time(&t);
    int rand_number = (current_time*(seed+2451*732)) % (max_range+1-min_range) + min_range ;
    return rand_number;
}

int main(int argc, char *argv[]){
    if(argc!=4){
        printf("You have to enter 3 arguments, you have entered %d arguments\n", argc-1);
        return 0;
    }
    //printf("Argc: %d\n", argc);

    int PORT_UDP=atoi(argv[1]); //requestes to file3
    char *IP=argv[2];           //requestes to file3
    int PORT_TCP=atoi(argv[3]); //requestes from cli3


    char send_txt[MAX_LINE];
    char recv_txt[MAX_LINE];
    char textin[MAX_LINE];

    int listen_fd, comm_fd;

    printf("Client side\n");

    // Here we create the conection with the server (file3.c), we recieve the number of lines, 
    // we send a random number in that range and we recieve a number from the server

    printf("Connecting to %s:%d\n", IP, PORT_UDP);
    
    int sock_fd;
    struct sockaddr_in servaddr;

    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0){
        perror("socket");
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT_UDP);

    if(inet_pton(AF_INET, IP, &(servaddr.sin_addr))!= 1) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return 1;
    }

    bzero(recv_txt, MAX_LINE);
    bzero(send_txt, MAX_LINE);

    if(read(sock_fd, recv_txt, MAX_LINE) < 0){
        perror("read");
        return 1;
    }

    int result = atoi(recv_txt);
    int random_number = random_number_gen(0, result, 0);
    sprintf(send_txt, "%d", random_number);

    if (write(sock_fd, send_txt, strlen(send_txt)) < 0) {
        perror("write");
        return 1;
    }

    printf("Respuesta servidor: %s\n",recv_txt);

    random_number = atoi(recv_txt);


    printf("Server side\n");

    listen_fd=socket(AF_INET, SOCK_STREAM, 0); 

    bzero(&servaddr, sizeof(servaddr)); 

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT_TCP);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);


    printf("Waiting for connection on 127.0.0.1:%d\n", PORT_TCP);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listen_fd, 1);

    comm_fd=accept(listen_fd, NULL, NULL);
    
    int totaliterations=0, min=0, num, randnum;

    do{
        bzero(textin,MAX_LINE);
        read(comm_fd,textin,MAX_LINE);
        num = atoi(textin);
        //printf("Text input: %d\n", num);

        result = compared(randnum, num);
        char textout[5];
        sprintf(textout, "%d", result);
        int chrs=strlen(textout);
        //printf("Text output: %s\n", textout);
        //printf("\n\n");

        write(comm_fd, textout, chrs);
        totaliterations++;
    }while(result!='0');
    printf("Total iterations: %d\n", totaliterations);
    return(0);
}