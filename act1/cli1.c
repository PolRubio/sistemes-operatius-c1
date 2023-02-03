#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h> // inet_pton()
#include <unistd.h> // read, write, close

#include <stdbool.h>

#define MAX_LINE 100
#define PORT 8080

int main(int argc, char **argv){   
    bool game=true, verify;
    int sock_fd, send_num, recv_num, start_r=0, end_r=100, num;
    struct sockaddr_in servaddr;

    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0){
        perror("socket");
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr))!= 1) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return 1;
    }

    while(game){
        verify=false;

        bzero(recv_num, sizeof(int));

        printf("take a guess [%d,%d]: ",start_r,end_r);
        while(!verify){
            bzero(send_num, sizeof(int));
            fgets(send_num,sizeof(int),stdin);
        }

        if(write(sock_fd, send_num, sizeof(int))){
            perror("write");
            return 1;
        }

        if(read(sock_fd, recv_num, sizeof(int)) < 0) {
            perror("read");
            return 1;
        }

        printf("received - %s\n",recv_num);

        // if(recv_num==0){
        //     printf("GUESSED CORRECTLY.");
        //     game=false;
        // } else if(recv_num>0){
        //     start_r=send_num;
        // } else {
        //     end_r=send_num;
        // }

        // rewrite
    }
}