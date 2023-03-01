#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define DEFAULT_PORT 8888
#define MAX_PORT 65535

int32_t compared(int32_t randnum, int32_t num){
    int32_t result=num-randnum;
    return (result<0)?-1:(result>0)?1:0;
}

int random_number_gen(int min_range, int max_range, int seed){
    time_t t;
    long long int current_time = time(&t);
    return (current_time*(seed+2451*732)) % (max_range+1-min_range) + min_range;
}

int main(int argc, char *argv[]){
    if(argc>2){
        printf("Too many arguments\nMaximum 1 argument, you have entered %d arguments\n", argc-1);
        return(0);
    }

    uint32_t 
        recv_value, 
        send_value;

    int32_t 
        recv_num, 
        min=0, 
        max=100, 
        result;

    int 
        port=(argc==2)?atoi(argv[1]):DEFAULT_PORT,
        listen_fd, 
        comm_fd,
        totaliterations=0,
        randnum=0;
        
    struct sockaddr_in servaddr;
    
    if(port>MAX_PORT || port<=0){
        printf("that port doesn't exists!\n");
        return(0);
    }

    printf("Server side\n");

    listen_fd=socket(AF_INET, SOCK_STREAM, 0); 

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    printf("Waiting for connection on port %d\n" , port);
    listen(listen_fd, 1);
    
    while(1){
        comm_fd=accept(listen_fd, NULL, NULL);

        randnum=random_number_gen(min, max, totaliterations);
        printf("\nrandom number: %d\n\n", randnum);
        
        do{
            read(comm_fd, &recv_value, sizeof(uint32_t));
            recv_num=(int32_t) ntohl(recv_value);

            printf("\trecieved input: %d\n", recv_num);

            result=compared(randnum, recv_num);
            send_value=htonl((uint32_t) result);

            printf("\tsending: %d\n\n", result);
            write(comm_fd, &result, sizeof(uint32_t));

            totaliterations++;
        }while(result!=0);
    }
    close(listen_fd);
}