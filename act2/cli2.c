#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MINIMUM 0
#define MAXIMUM 100

#define DEFAULT_PORT 8888
#define DEFAULT_IP "127.0.0.1"

#define MAX_PORT 65535


int main(int argc, char *argv[]){       
    if(argc>3){
        fprintf(stderr,"Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }

    int
        sock_fd,
        iteration=0,
        port=(argc>=2)?atoi(argv[1]):DEFAULT_PORT;

    char *ip_address=(argc>2)?argv[2]:DEFAULT_IP;

    int32_t
        send_num=-1,
        recv_num=0,

        min=MINIMUM, 
        max=MAXIMUM,
        current_guess=max/2;

    uint32_t 
        send_value=0,
        recv_value=0;    

    struct sockaddr_in servaddr;

    if(!(port<=MAX_PORT && port>=0)){
        fprintf(stderr,"Invalid port number\n");
        exit(0);
    }

    printf("Client side\n");
    printf("Connecting to %s:%d\n", ip_address, port);

    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0){
        perror("socket");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);

    if(inet_pton(AF_INET, ip_address, &(servaddr.sin_addr))!= 1) {
        perror("inet_pton");
        exit(0);
    }

    int connect_status=connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(connect_status < 0) {
        perror("connect");
        exit(0);
    }

    do{
        iteration++;
        
        printf("\n\npicked %d\n", current_guess);
        send_value=htonl((uint32_t)current_guess);
        if (write(sock_fd, &send_value, sizeof(uint32_t)) < 0) {
            perror("write");
            exit(0);
        }

        if (read(sock_fd, &recv_value, sizeof(uint32_t)) < 0) {
            perror("read");
            exit(0);
        }
        recv_num=ntohl((int32_t)recv_value);

        if(max-min==1){
            printf("reached the limit!\n");
            current_guess=max;
            break;
        }else if(recv_num==0){
            printf("\nrandom number is equal to %d\n", current_guess);
            break;
        }else{
            if(recv_num>0){
                printf("\trandom number is less than %d\n", current_guess);
                max=current_guess;
            }else{
                printf("\trandom number is greater than %d\n", current_guess);
                min=current_guess;
            }
            current_guess=(max+min)/2;
        }
        send_num=0;
    }while(recv_num!=0);

    printf("\tnumber: %d\n\tguesses: %d\n", current_guess, iteration);

    close(sock_fd);
    printf("goodbye.\n");
}
