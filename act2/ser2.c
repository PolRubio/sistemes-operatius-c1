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
    int *tmp=malloc(initial_size*sizeof(int));
    if(!tmp) {
        printf("error happend during the creation of the array.");
        exit(0);
    }

    a->array=tmp;
    a->used=0;
    a->size=initial_size;
}

void insert_array(Array *a, int element){
    if(a->used==a->size){
        a->size*=2; // each time the array fills, expand it 100%. probably not the most optimzed value, but it will work.
        int *tmp=realloc(a->array,a->size*sizeof(int));
        if(!tmp) {
            printf("error happend during the extensioN of the array.");
            exit(0);
        }
        a->array=tmp;
    }
    a->array[a->used++]=element;
}

void free_array(Array *a){
    free(a->array);
    a->array=NULL;
    a->used=a->size=0;
}
// END


void get_file_props(Array *a, char *filename){
    FILE *file=fopen(filename,"r");
    if(file==NULL){
       printf("Error! opening file: %s", filename);
       return(0);
    }
    printf("file open\n");

    int counter=0;
    for (char c=getc(file); c!=EOF; c=getc(file)){
        if(c=='\n'){
            insert_array(a,counter%100);
            counter=0;
        }
        counter++;
    }
    
    printf("file read\n");
    fclose(file);  
}

int main(int argc, char *argv[]){
    if(argc>3){
        printf("Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        return 0;
    }if(argc<2){
        printf("Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
        return 0;
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
        port=(argc>2)?atoi(argv[2]):DEFAULT_PORT,
        listen_fd,
        comm_fd,
        totaliterations=0,
        randnum=0,
        charcount=0,
        iterations=0, 
        min=0, 
        num, 
        result, 
        randnum;

    char 
        *filename=argv[1],
        *myString,
        c;

    struct sockaddr_in servaddr;

    Array array_holder;
    init_array(&array_holder, INITIAL_ARRAY_SIZE);

    if(port>MAX_PORT || port<=0){
        printf("that port doesn't exists!\n");
        return 0;
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
        printf("Waiting for connection on 127.0.0.1:%d...\n", port);
        comm_fd=accept(listen_fd, NULL, NULL);

        randnum=array_holder.array[random_number_gen(0, array_holder.used, totaliterations)];
        printf("\nrandom number: %d\n\n", randnum);

        do{
            read(comm_fd, &recv_value, sizeof(recv_value));
            recv_num=(int32_t) ntohl(recv_value);
            
            printf("\trecieved input: %d\n", recv_num);

            result=compared(randnum, recv_num);
            send_value=htonl((uint32_t) result);

            printf("\tsending: %d\n\n", result);
            write(comm_fd, &result, sizeof(result));

            totaliterations++;
        }while(result!=0);
        
        printf("Total iterations: %d\n", totaliterations);
    }
    close(listen_fd);
    free_array(&array_holder);
}