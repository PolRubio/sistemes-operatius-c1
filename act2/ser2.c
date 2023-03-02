#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define MAXIMUM 100

#define DEFAULT_PORT 8888
#define MAX_PORT 65535

#define INITIAL_ARRAY_SIZE 5


int32_t compared(int32_t randnum, int32_t num){
  int32_t result=num-randnum;
  return (result<0)?-1:(result>0)?1:0;
}

int random_number_gen(int min_range, int max_range, int seed){
    time_t t;
    long long int current_time=time(&t);
    return (current_time*(seed+2451*732)) % (max_range+1-min_range) + min_range;
}

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
            printf("error happend during the extension of the array.");
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
        fprintf(stderr,"Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        return(0);
    }if(argc<2){
        fprintf(stderr,"Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
        return(0);
    }

    uint32_t
        recv_value,
        send_value;

    int32_t
        recv_num,
        result;

    int
        port=(argc>2)?atoi(argv[2]):DEFAULT_PORT,
        listen_fd,
        comm_fd,
        randnum=0,
        iterations=0,
        general_iterations=0;

    char *filename=argv[1];

    struct sockaddr_in servaddr;

    Array array_holder;
    init_array(&array_holder, INITIAL_ARRAY_SIZE);

    if(port>MAX_PORT || port<=0){
        fprintf(stderr, "that port doesn't exists!\n");
        return(0);
    }

    printf("Server side\n");

    listen_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd<0){
        perror("socket");
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);
    
    get_file_props(&array_holder, filename);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listen_fd, 1);    

    while (1){
        printf("\nWaiting for connection on 127.0.0.1:%d...\n", port);

        comm_fd=accept(listen_fd, NULL, NULL);
        if(comm_fd<0){
            fprintf(stderr, "Failed to accept the connection:%d\n", comm_fd);
        }

        int indx=random_number_gen(0, array_holder.used, general_iterations++);
        randnum=array_holder.array[indx];
        printf("\nrandom number: %d\n\n", randnum);

        iterations=0;
        do{
            if(read(comm_fd, &recv_value, sizeof(recv_value))>0){
                perrro("read");
                return 0;
            }
            recv_num=(int32_t) ntohl(recv_value);
            printf("\trecieved input: %d\n", recv_num);

            result=compared(randnum, recv_num);
            send_value=htonl((uint32_t) result);
            printf("\tsending: %d\n\n", result);

            if(write(comm_fd, &result, sizeof(result))){
                perror("write");
                return 0;
            }

            iterations++;
        }while(result!=0);
        
        printf("Total iterations: %d\n", iterations);
    }

    close(listen_fd);
    free_array(&array_holder);
}