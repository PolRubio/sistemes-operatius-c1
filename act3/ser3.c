#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define MAX_PORT 65535

int32_t compared(int randnum, int num){
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

    uint32_t
        send_value=0,
        recv_value=0;

    int32_t
        send_num=-1,
        recv_num=0,
        result;

    int 
        port_udp=atoi(argv[1]), //requestes to file3
        port_tcp=atoi(argv[3]), //requestes from cli3
        sock_fd,
        listen_fd,
        comm_fd,
        random_number,
        numlieas,
        totaliterations=0,
        randnum=0;

    char
        *ip_address=argv[2],          //requestes to file3
        send_txt[MAX_LINE],
        recv_txt[MAX_LINE],
        textin[MAX_LINE];

    struct sockaddr_in servaddr;

    if(port_udp>MAX_PORT || port_udp<=0 || port_tcp>MAX_PORT || port_tcp<=0){
        printf("that port doesn't exists!\n");
        return(0);
    }
    
    printf("Client side\n");

    // Here we create the conection with the server (file3.c), we recieve the number of lines, 
    // we send a random number in that range and we recieve a number from the server

    printf("Connecting to %s:%d\n", ip_address, port_udp);
    

    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0){
        perror("socket");
        return(1);
    }

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port_udp);

    if(inet_pton(AF_INET, ip_address, &(servaddr.sin_addr))!= 1) {
        perror("inet_pton");
        return(1);
    }

    if (connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return(1);
    }

    
    if (read(sock_fd, &recv_value, sizeof(uint32_t)) < 0) {
        perror("read");
        return(0);
    }
    numlieas=ntohl((int32_t)recv_value);
    printf("\trecieved input: %d\n", numlieas);

    send_num=(int32_t) random_number_gen(0, numlieas, 0);
    printf("\n\npicked %d\n", send_num);
    send_value=htonl((uint32_t)send_num);
    if (write(sock_fd, &send_value, sizeof(uint32_t)) < 0) {
        perror("write");
        return(0);
    }

    if (read(sock_fd, &recv_value, sizeof(uint32_t)) < 0) {
        perror("read");
        return(0);
    }
    random_number=ntohl((int32_t)recv_value);
    printf("\trecieved input: %d\n", random_number);

    
    printf("Server side\n");

    listen_fd=socket(AF_INET, SOCK_STREAM, 0); 

    bzero(&servaddr, sizeof(servaddr)); 

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port_tcp);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);

    printf("Waiting for connection on 127.0.0.1:%d\n", port_tcp);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listen_fd, 1);

    while(1){
        comm_fd=accept(listen_fd, NULL, NULL);
        
        send_num=(int32_t) random_number_gen(0, numlieas, 0);
        printf("\n\npicked %d\n", send_num);
        send_value=htonl((uint32_t)send_num);
        if (write(sock_fd, &send_value, sizeof(uint32_t)) < 0) {
            perror("write");
            return(0);
        }

        if (read(sock_fd, &recv_value, sizeof(uint32_t)) < 0) {
            perror("read");
            return(0);
        }
        random_number=ntohl((int32_t)recv_value);
        printf("\trecieved input: %d\n", random_number);
        
        do{
            read(comm_fd, &recv_value, sizeof(uint32_t));
            recv_num=(int32_t) ntohl(recv_value);
            
            printf("\t\trecieved input: %d\n", recv_num);

            result=compared(randnum, recv_num);
            send_value=htonl((uint32_t) result);

            printf("\t\tsending: %d\n\n", result);
            write(comm_fd, &result, sizeof(uint32_t));

            totaliterations++;
        }while(result!='0');
    
        printf("Total iterations: %d\n", totaliterations);
    }
    close(listen_fd);
}