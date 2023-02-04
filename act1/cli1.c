#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h> // inet_pton()
#include <unistd.h> // read, write, close


#define MAX_LINE 100
#define PORT 8080

int main(int argc, char **argv){   
    int sock_fd;

    char send_txt[MAX_LINE];
    char recv_txt[MAX_LINE];

    struct sockaddr_in servaddr;

    printf("Client side\n");

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

    while(1){
        bzero(recv_txt, MAX_LINE);
        bzero(send_txt, MAX_LINE);

        printf("Max number to generate: ");
        fgets(send_txt,MAX_LINE,stdin);
        
        if (write(sock_fd, send_txt, strlen(send_txt)) < 0) {
            perror("write");
            return 1;
        }

        if (read(sock_fd, recv_txt, MAX_LINE) < 0) {
            perror("read");
            return 1;
        }
        printf("Random number generated: %s\n",recv_txt);
        printf("\n\n");
    }
}
