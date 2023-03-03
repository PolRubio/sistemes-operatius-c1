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

#define CONN_KEY 1



int32_t compared(int randnum, int num){
    // COMPARES THE RANDOM NUMBER WITH THE NUMBER RECEIVED FROM THE CLIENT
    if(num>randnum) return 1;
    else if(num<randnum) return -1;
    else return 0;
}

int random_number_gen(int min_range, int max_range, int seed){
    // random number generator

    time_t t;
    long long int current_time = time(&t);
    int rand_number = (current_time*(seed+2451*732)) % (max_range+1-min_range) + min_range ;
    return rand_number;
}

void port_checker(int udp_port, int tcp_port){
    // checking port validity

    if(!(udp_port<=MAX_PORT && udp_port>=0 && tcp_port<=MAX_PORT && tcp_port>=0)){
        fprintf(stderr,"Invalid port number\n");
        exit(0);
    }else if(udp_port==tcp_port){
        fprintf(stderr,"udp and tcp port can't be the same! (%d=%d)",tcp_port,udp_port);
        exit(0);
    }
}

int32_t get_lines_num(struct sockaddr_in *udp_servaddr, int *udp_sock_fd){
    // get the number of lines from file3 udp server

    int 
        udp_sent,
        udp_received;
    uint32_t 
        secret=htonl((uint32_t) CONN_KEY),
        recv_value;
    unsigned int udp_servaddr_size=sizeof(*udp_servaddr);

    // "awaking" file3 to recieve the number of lines. using sendto because file3 is an UDP server.
    udp_sent=sendto(*udp_sock_fd,&secret,sizeof(uint32_t),0,(struct sockaddr *) udp_servaddr,udp_servaddr_size);
    if(udp_sent<0){
        perror("sendto");
        exit(0);
    }

    // wait for the number of lines from the file3 server and write it to recv_value
    udp_received=recvfrom(*udp_sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) udp_servaddr,&udp_servaddr_size);
    if(udp_received<0){
        perror("recvfrom");
        exit(0);
    }

    return((int32_t) ntohl(recv_value));
}

int get_secret(int received_lines_num, int *udp_sock_fd, struct sockaddr_in *udp_servaddr){
    // pick a random file line number and send it to file3 server to recieve back the number of characters%MAXIMUM (100). 
    
    int 
        random_line=(int32_t) random_number_gen(0, received_lines_num, 1),
        udp_sent,
        udp_received;
    uint32_t 
        encoded_line=htonl((uint32_t)random_line), // host byte order to network byte order
        recv_value;
    unsigned int udp_servaddr_size=sizeof(*udp_servaddr);
    
    printf("\n\npicked a random line %d [0,%d]\n", random_line, received_lines_num);

    // sending the random line number to UDP server
    udp_sent=sendto(*udp_sock_fd,&encoded_line,sizeof(encoded_line),0,(struct sockaddr *) udp_servaddr,udp_servaddr_size);
    if(udp_sent<0){
        perror("sendto");
        exit(0);
    }

    // receive the number of characters%MAXIMUM (100).
    udp_received=recvfrom(*udp_sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) udp_servaddr,&udp_servaddr_size);
    if(udp_received<0){
        perror("recvfrom");
        exit(0);
    }
    
    return((int32_t) ntohl(recv_value)); // network byte order to host byte order
}

void setup_udp_connection(int *udp_sock_fd, struct sockaddr_in *udp_servaddr, int udp_port,char *ip_address){
    // creating an UDP connection to ip_address:udp_port 

    *udp_sock_fd=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    // CHECKING IF THE SOCKET IS CREATED SUCCESSFULLY
    if (*udp_sock_fd<0){
        perror("socket");
        exit(0);
    }

    udp_servaddr->sin_family=AF_INET;
    udp_servaddr->sin_port=htons(udp_port);
    udp_servaddr->sin_addr.s_addr=inet_addr(ip_address);
}

void setup_tcp_connection(int *tcp_sock_fd, struct sockaddr_in *tcp_servaddr, int tcp_port){
    // creating a TCP server

    *tcp_sock_fd=socket(AF_INET, SOCK_STREAM, 0);

    // CHECKING IF THE SOCKET IS CREATED SUCCESSFULLY
    if(*tcp_sock_fd<0){
        perror("socket");
        exit(0);
    }

    // INITIALIZING THE SOCKET ADDRESS STRUCTURE
    tcp_servaddr->sin_family=AF_INET;
    tcp_servaddr->sin_port=htons(tcp_port);
    tcp_servaddr->sin_addr.s_addr=htons(INADDR_ANY);

    // binding the port unmber
    if(bind(*tcp_sock_fd, (struct sockaddr *) tcp_servaddr, sizeof(*tcp_servaddr))<0){
        perror("bind");
        exit(0); 
    }

    // preaparing to accept connections
    if(listen(*tcp_sock_fd, 1)<0){
        perror("listen");
        exit(0);
    }
}

int main(int argc, char *argv[]){
    // CHECKING THE NUMBER OF ARGUMENTS
    if(argc!=4){
        printf("You have to enter 3 arguments, you have entered %d arguments\n", argc-1);
        return 0;
    }

    // VARIABLES DECLARATION AND INITIALIZATION
    uint32_t
        send_value=0,
        recv_value=0;

    int32_t
        feedback=-1,
        recv_num=0;

    int 
        udp_port=atoi(argv[1]), //requestes to file3
        udp_sock_fd,
        udp_sent,
        udp_received,
        
        tcp_port=atoi(argv[3]), //requestes from cli3
        tcp_sock_fd,

        comm_fd,
        secret_number,
        received_lines_num,
        totaliterations=0;

    char *ip_address=argv[2];

    // STRUCTURES DECLARATION AND INITIALIZATION
    struct sockaddr_in udp_servaddr,tcp_servaddr;

    // CLEARING THE STRUCTURES
    memset(&tcp_servaddr, 0, sizeof(tcp_servaddr));
    memset(&udp_servaddr, 0, sizeof(udp_servaddr));

    // CHECK THE VALIDITY OF PORTS
    port_checker(udp_port,tcp_port);

    // SETTING UP UDP CONNECTION
    setup_udp_connection(&udp_sock_fd,&udp_servaddr,udp_port,ip_address);

    // GET NUMBER OF LINES FROM FILE3
    received_lines_num=get_lines_num(&udp_servaddr,&udp_sock_fd);

    // SETTING UP TCP CONNECTION
    setup_tcp_connection(&tcp_sock_fd,&tcp_servaddr,tcp_port);

    while(1){
        printf("Waiting for connection on 127.0.0.1:%d\n", tcp_port);

        // wait for clients
        comm_fd=accept(tcp_sock_fd, NULL, NULL);
        if(comm_fd<0){
            fprintf(stderr,"Failed to accept the connection:%d\n",comm_fd);
        }

        // Get the number of characters.
        secret_number=get_secret(received_lines_num,&udp_sock_fd,&udp_servaddr);

        totaliterations=0;
        do{
            // wait for user guess of TCP connection
            if(read(comm_fd, &recv_value, sizeof(recv_value))<0){
                perror("read");
                return 0;
            }

            // CONVERT FROM NETWORK BYTE ORDER TO HOST BYTE ORDER
            recv_num=(int32_t) ntohl(recv_value);
            printf("\trecieved guess: %d\n", recv_num);

            // generating feedback
            feedback=compared(secret_number, recv_num);
            send_value=htonl((uint32_t) feedback);
            printf("\tsending result: %d\n\n", feedback);

            // give back feedback on the guessed number,
            if(write(comm_fd, &feedback, sizeof(feedback))<0){
                perror("write");
                return 0;
            }

            totaliterations++;
        }while(feedback!=0);
    
        printf("Total iterations: %d\n", totaliterations);
    }

    // closing udp socket
    close(udp_sock_fd);

    // closing tcp socket
    close(tcp_sock_fd);
}
