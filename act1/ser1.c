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

#define MAXMIMUM 100
#define MINIMUM 0

int32_t compared(int32_t randnum, int32_t num){
    /**
     * COMPARES THE RANDOM NUMBER WITH THE NUMBER RECEIVED FROM THE CLIENT
     **/
    int32_t result=num-randnum;
    return (result<0)?-1:(result>0)?1:0;
}

int random_number_gen(int min_range, int max_range, int seed){
    /**
     * GENERATES A PSEUDO RANDOM NUMBER USING THE CURRENT TIME
     **/
    time_t t;
    long long int current_time = time(&t);
    return (current_time*(seed+2451*732)) % (max_range+1-min_range) + min_range;
}

int main(int argc, char *argv[]){
    // CHECKS IF THE PORT IS VALID
    if(argc>2){
        fprintf(stderr, "Too many arguments\nMaximum 1 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }

    // VARIABLES DECLARATION AND INITIALIZATION
    uint32_t 
        recv_value, 
        send_value;

    int32_t 
        recv_num, 
        min=MINIMUM, 
        max=MAXMIMUM, 
        result;

    int 
        port=(argc==2)?atoi(argv[1]):DEFAULT_PORT,
        sock_fd, 
        comm_fd,
        totaliterations=0,
        randnum=0;
        
    struct sockaddr_in servaddr;

    // CHECKS IF THE PORT IS VALID    
    if(!(port<=MAX_PORT && port>=0)){
        fprintf(stderr,"Invalid port number\n");
        exit(0);
    }

    printf("Server side\n");

    // CREATE THE SOCKET AND HANDLE ERRORS
    sock_fd=socket(AF_INET, SOCK_STREAM, 0); 
    if(sock_fd<0){
        fprintf(stderr,"Failed to create socket: %s\n", strerror(sock_fd));
        return 0;
    }

    // BIND THE SOCKET TO THE PORT AND HANDLE ERRORS
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);
    if(bind(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
        perror("bind");
        return 0;
    }

    // LISTEN FOR CONNECTIONS AND HANDLE ERRORS
    if(listen(sock_fd, 1)<0){
        perrro("listen");
        return 0;
    }
    
    while(1){
        printf("Waiting for connection on port %d\n" , port);

        // ACCEPT CONNECTIONS AND HANDLE ERRORS
        comm_fd=accept(sock_fd, NULL, NULL);
        if(comm_fd<0){
            perror("accept connection");
            return 0;
        }

        // GENERATE A RANDOM NUMBER
        randnum=random_number_gen(min, max, totaliterations);
        printf("\nrandom number: %d\n\n", randnum);
        
        do{
            // READ THE INPUT FROM THE CLIENT AND HANDLE ERRORS
            if(read(comm_fd, &recv_value, sizeof(uint32_t))<0){
                perror("read");
                return 0;
            }

            // CONVERT FROM NETWORK BYTE ORDER TO HOST BYTE ORDER
            recv_num=(int32_t) ntohl(recv_value);

            printf("\trecieved input: %d\n", recv_num);

            result=compared(randnum, recv_num);

            // CONVERT FROM HOST BYTE ORDER TO NETWORK BYTE ORDER
            send_value=htonl((uint32_t) result);

            printf("\tsending: %d\n\n", result);

            // SEND THE RESULT TO THE CLIENT AND HANDLE ERRORS
            if(write(comm_fd, &result, sizeof(uint32_t))<0){
                perror("wite");
                return 0;
            }

            totaliterations++;
        }while(result!=0);
        
        printf("Total iterations: %d\n", totaliterations);
    }
    // CLOSES THE CONNECTION AND THE SOCKET
    close(sock_fd);
}