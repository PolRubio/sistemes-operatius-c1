#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define DEFAULT_PORT 9999
#define MAX_PORT 65535

#define MAXIMUM 100

#define INITIAL_ARRAY_SIZE 5

// DYNAMIC ARRAY
typedef struct{
    int *array; // array of elements
    size_t used; // how much positions are filled
    size_t size; // current size of the array
} Array;

void init_array(Array *a, size_t initial_size){
    // initialize the array, by exmpanding the array size 

    a->array=malloc(initial_size*sizeof(int));
    if(a->array==NULL){
        perror("malloc");
        exit(0);
    }

    a->used=0;
    a->size=initial_size;
}

void insert_array(Array *a, int element){
    // if the array is completly filled, expand it.
    if(a->used==a->size){
        a->size*=2; // each time the array fills, expand it 100%. probably not the most optimzed value, but it will work.
        int *tmp=realloc(a->array,a->size*sizeof(int)); // expand the memory size
        if(!tmp) {
            fprintf(stderr,"error happend during the extension of the array.");
            exit(0);
        }
        a->array=tmp;
    }
    a->array[a->used++]=element; // insert element
}

void free_array(Array *a){
    // clearing used memory space
    memset(a->array, 0, sizeof(a->array));
    free(a->array);

    a->array=NULL;
    a->used=a->size=0;
}
// END

void get_file_props(Array *a, char *filename){
    // getting the number of lines and the steam
    FILE *file=fopen(filename,"r");
    if(file==NULL){
       fprintf(stderr, "Error! opening file: %s", filename);
       exit(0);
    }

    int counter=0;
    for (char c=getc(file); c!=EOF; c=getc(file)){
        if(c=='\n'){
            insert_array(a,counter%MAXIMUM);
            counter=0;
        }
        counter++;
    }
    fclose(file);
}

void send_file_lines(struct sockaddr_in *clientaddr,int *sock_fd, int used){
    unsigned int clientaddr_len=sizeof(*clientaddr);
    int
        received,
        sent;
    uint32_t 
        send_value,
        recv_value;

    received=recvfrom(*sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) &clientaddr,&clientaddr_len);
    if (received<0) {
        perror("Error receiveing from client");
        exit(0);
    }
    printf("got %d\n",(int) ntohl(recv_value));

    send_value=htonl((uint32_t) used);
    printf("sending lines: %d\n\n", used);

    sent=sendto(*sock_fd,&send_value,received,0,(struct sockaddr *) &clientaddr,clientaddr_len);
    if(sent<0){
        perror("send error");
        exit(0);
    }
}

void validate_num_args(int argc){
    if(argc>3){
        printf("Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }
    if(argc<2){
        printf("Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }
}

void setup_udp_connection(int *sock_fd, struct sockaddr_in *servaddr, int port){
    *sock_fd=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*sock_fd<0){
        perror("socket");
        exit(0);
    }

    servaddr->sin_family=AF_INET;
    servaddr->sin_port=htons(port);
    servaddr->sin_addr.s_addr=htons(INADDR_ANY);
    
    if(bind(*sock_fd, (struct sockaddr *) servaddr, sizeof(*servaddr))<0){
        perror("bind");
        exit(0);
    }
}

int main(int argc, char *argv[]){
    validate_num_args(argc);

    uint32_t
        recv_value,
        send_value;

    int32_t
        recv_num,
        max=MAXIMUM;

    int
        port=(argc>2)?atoi(argv[2]):DEFAULT_PORT,
        sock_fd,
        comm_fd,
        received,sent;

    char *filename=argv[1];

    struct sockaddr_in servaddr, clientaddr;

    Array array_holder;
    init_array(&array_holder, INITIAL_ARRAY_SIZE);

    // CHECKING PORTS
    if(!(port<=MAX_PORT && port>=0)){
        printf("Invalid port number\n");
        exit(0);
    }
    // END


    // CLEARING VARIABLES
    memset(&servaddr,0,sizeof(servaddr));
    memset(&clientaddr,0,sizeof(clientaddr));

    memset(&send_value,0,sizeof(send_value));
    memset(&recv_value,0,sizeof(recv_value));
    // END

    unsigned int clientaddr_len=sizeof(clientaddr);


    printf("Server side\n");

    // CREATE UDP CONNECTION
    setup_udp_connection(&sock_fd,&servaddr,port);

    // READ FILE FROM ARGS
    get_file_props(&array_holder,filename);

    printf("\nWaiting for initial connection on 127.0.0.1:%d...\n", port);

    // INITIAL CONNECTION (wait for UDP client to contact and send number of lines.)
    send_file_lines(&clientaddr,&sock_fd, (int) array_holder.used);


    while(1){
        // RECEIVE RANDOM NUMBER OF LINE
        received=recvfrom(sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) &clientaddr,&clientaddr_len);
        if (received<0) {
            perror("Error receiving from client");
            exit(0);
        }

        recv_num=((int32_t) ntohl(recv_value)) % array_holder.used;
        printf("\trecieved line num: %d\n", recv_num);

        // getting the number of characters on recieved line number.
        send_value=htonl((uint32_t) array_holder.array[recv_num]);
        printf("\tsending chars: %d\n\n", array_holder.array[recv_num]);

        // SEND NUMBER OF CHARS
        sent=sendto(sock_fd,&send_value,received,0,(struct sockaddr *) &clientaddr,clientaddr_len);
        if(sent<0){
            perror("send");
            exit(0);
        }
    }
    close(sock_fd);

    free_array(&array_holder);
}