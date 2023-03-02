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
    int *array;
    size_t used;
    size_t size;
} Array;

void init_array(Array *a, size_t initial_size){
    a->array=malloc(initial_size*sizeof(int));
    a->used=0;
    a->size=initial_size;
}

void insert_array(Array *a, int element){
    if(a->used==a->size){
        a->size*=2; // each time the array fills, expand it 100%. probably not the most optimzed value, but it will work.
        int *tmp=realloc(a->array,a->size*sizeof(int));
        if(!tmp) {
            fprintf(stderr,"error happend during the extension of the array.");
            exit(0);
        }
        a->array=tmp;
    }
    a->array[a->used++]=element;
}

void free_array(Array *a){
    memset(a->array, 0, sizeof(a->array));
    free(a->array);

    a->array=NULL;
    a->used=a->size=0;
}
// END

void get_file_props(Array *a, char *filename){
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

int main(int argc, char *argv[]){
    if(argc>3){
        printf("Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }
    if(argc<2){
        printf("Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }

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

    if(port>MAX_PORT || port<=0){
        printf("that port doesn't exists!\n");
        exit(0);
    }

    printf("Server side\n");

    sock_fd=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock_fd<0){
        perror("socker");
        exit(0);
    }

    memset(&servaddr,0,sizeof(servaddr));
    memset(&clientaddr,0,sizeof(clientaddr));

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);

    get_file_props(&array_holder,filename);

    if(bind(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
        perror("bind");
        exit(0);
    }

    printf("\nWaiting for initial connection on 127.0.0.1:%d...\n", port);

    // INITIAL CONNECTION (wait for UDP client to contact and send number of lines.)
    unsigned int clientaddr_len=sizeof(clientaddr);
    received=recvfrom(sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) &clientaddr,&clientaddr_len);
    if (received<0) {
        perror("Error receiveing from client");
        exit(0);
    } else printf("got %d\n",(int) ntohl(recv_value));

    send_value=htonl((uint32_t) array_holder.used);
    printf("sending lines: %d\n\n", (int)array_holder.used);

    sent=sendto(sock_fd,&send_value,received,0,(struct sockaddr *) &clientaddr,clientaddr_len);
    if(sent<0){
        perror("send error");
        exit(0);
    }
    // END

    while(1){
        memset(&send_value,0,sizeof(send_value)); // necessary?
        memset(&recv_value,0,sizeof(recv_value)); // necessary?
        

        received=recvfrom(sock_fd,&recv_value,sizeof(recv_value),0,(struct sockaddr *) &clientaddr,&clientaddr_len);
        if (received<0) {
            perror("Error receiveing from client");
            exit(0);
        }

        recv_num=((int32_t) ntohl(recv_value)) % array_holder.used;
        printf("\trecieved line num: %d\n", recv_num);

        send_value=htonl((uint32_t) array_holder.array[recv_num]);
        printf("\tsending chars: %d\n\n", array_holder.array[recv_num]);

        sent=sendto(sock_fd,&send_value,received,0,(struct sockaddr *) &clientaddr,clientaddr_len);
        if(sent<0){
            perror("send");
            exit(0);
        }
    }
    close(sock_fd);
}