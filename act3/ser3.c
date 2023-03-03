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

void port_checker(int udp_port, int tcp_port){
    if(!(udp_port<=MAX_PORT && udp_port>=0 && tcp_port<=MAX_PORT && tcp_port>=0)){
        fprintf(stderr,"Invalid port number\n");
        exit(0);
    }else if(udp_port==tcp_port){
        fprintf(stderr,"udp and tcp port can't be the same! (%d=%d)",tcp_port,udp_port);
        exit(0);
    }
}

int32_t get_lines_num(struct sockaddr_in *udp_servaddr, int *udp_sock_fd){
    int 
        udp_sent,
        udp_received;
    uint32_t 
        secret=htonl((uint32_t) CONN_KEY),
        recv_value;
    unsigned int udp_servaddr_size=sizeof(*udp_servaddr);

    udp_sent=sendto(*udp_sock_fd,&secret,sizeof(uint32_t),0,(struct sockaddr *) udp_servaddr,udp_servaddr_size);
    if(udp_sent<0){
        perror("sendto");
        exit(0);
    }

    udp_received=recvfrom(*udp_sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) udp_servaddr,&udp_servaddr_size);
    if(udp_received<0){
        perror("recvfrom");
        exit(0);
    }

    return((int32_t) ntohl(recv_value));
}

int get_secret(int received_lines_num, int *udp_sock_fd, struct sockaddr_in *udp_servaddr){
    
    int 
        random_line=(int32_t) random_number_gen(0, received_lines_num, 1),
        udp_sent,
        udp_received;
    uint32_t 
        encoded_line=htonl((uint32_t)random_line),
        recv_value;
    unsigned int udp_servaddr_size=sizeof(*udp_servaddr);
    
    printf("\n\npicked a random line %d [0,%d]\n", random_line, received_lines_num);

    udp_sent=sendto(*udp_sock_fd,&encoded_line,sizeof(encoded_line),0,(struct sockaddr *) udp_servaddr,udp_servaddr_size);
    if(udp_sent<0){
        perror("sendto");
        exit(0);
    }

    udp_received=recvfrom(*udp_sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) udp_servaddr,&udp_servaddr_size);
    if(udp_received<0){
        perror("recvfrom");
        exit(0);
    }
    
    return((int32_t) ntohl(recv_value));
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

    struct sockaddr_in udp_servaddr,tcp_servaddr;

    port_checker(udp_port,tcp_port);

    // SETTING UP UDP CONNECTION
    udp_sock_fd=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_sock_fd<0){
        perror("socket");
        return(1);
    }
    udp_servaddr.sin_family=AF_INET;
    udp_servaddr.sin_port=htons(udp_port);
    udp_servaddr.sin_addr.s_addr=inet_addr(ip_address);
    // END

    received_lines_num=get_lines_num(&udp_servaddr,&udp_sock_fd);

    // SETTING UP TCP CONNECTION
    tcp_sock_fd=socket(AF_INET, SOCK_STREAM, 0); 

    memset(&tcp_servaddr, 0, sizeof(tcp_servaddr)); 
    tcp_servaddr.sin_family=AF_INET;
    tcp_servaddr.sin_port=htons(tcp_port);
    tcp_servaddr.sin_addr.s_addr=htons(INADDR_ANY);

    if(bind(tcp_sock_fd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr))<0){
        perror("bind");
        exit(0); 
    }
    if(listen(tcp_sock_fd, 1)<0){
        perror("listen");
        exit(0);
    }
    // END

    while(1){
        printf("Waiting for connection on 127.0.0.1:%d\n", tcp_port);

        comm_fd=accept(tcp_sock_fd, NULL, NULL);
        if(comm_fd<0){
            fprintf(stderr,"Failed to accept the connection:%d\n",comm_fd);
        }

        // "GENERATE" "RANDOM" NUMBER 
        secret_number=get_secret(received_lines_num,&udp_sock_fd,&udp_servaddr);
        // END

        totaliterations=0;
        do{
            if(read(comm_fd, &recv_value, sizeof(recv_value))<0){
                perror("read");
                return 0;
            }
            recv_num=(int32_t) ntohl(recv_value);
            printf("\trecieved guess: %d\n", recv_num);

            send_num=compared(secret_number, recv_num);
            send_value=htonl((uint32_t) send_num);
            printf("\tsending result: %d\n\n", send_num);

            if(write(comm_fd, &send_num, sizeof(send_num))<0){
                perror("write");
                return 0;
            }

            totaliterations++;
        }while(send_num!=0);
    
        printf("Total iterations: %d\n", totaliterations);
    }
    
    close(udp_sock_fd);
    close(tcp_sock_fd);
}