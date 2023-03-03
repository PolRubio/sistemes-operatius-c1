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

// DYNAMIC ARRAY
typedef struct{
    int *array;
    size_t used;
    size_t size;
} Array;

void init_array(Array *a, size_t initial_size){
    /**
     * INITIALIZES THE ARRAY
     **/
    a->array=malloc(initial_size*sizeof(int));
    a->used=0;
    a->size=initial_size;
}

void insert_array(Array *a, int element){
    /**
     * INSERTS AN ELEMENT INTO THE ARRAY AND EXPANDS IT IF NEEDED
     **/
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
    /**
     * FREE THE ARRAY FROM MEMORY AND SETS THE VALUES TO 0 AND NULL
     **/
    memset(a->array, 0, sizeof(a->array));
    free(a->array);

    a->array=NULL;
    a->used=a->size=0;
}

void get_file_props(Array *a, char *filename){
    /**
     * OPEN THE FILE AND GETS THE NUMBER OF CHARACTERS IN EACH LINE AND INSERTS IT INTO THE ARRAY
     **/
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

void validate_argc(int argc){
    if(argc>3){
        fprintf(stderr,"Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }if(argc<2){
        fprintf(stderr,"Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
        exit(0);
    }
}

int main(int argc, char *argv[]){
    // CHECKS IF THE NUMBER OF ARGUMENTS IS VALID
    validate_argc(argc);

    // VARIABLES DECLARATION AND INITIALIZATION
    uint32_t
        recv_value,
        send_value;

    int32_t
        recv_num,
        result;

    int
        port=(argc>2)?atoi(argv[2]):DEFAULT_PORT,
        sock_fd,
        comm_fd,
        randnum=0,
        iterations=0,
        general_iterations=0;

    char *filename=argv[1];

    struct sockaddr_in servaddr;

    Array array_holder;
    init_array(&array_holder, INITIAL_ARRAY_SIZE);

    // CHECKS IF THE PORT NUMBER IS VALID
    if(!(port<=MAX_PORT && port>=0)){
        fprintf(stderr, "Invalid port number\n");
        exit(0);
    }

    printf("Server side\n");

    // CREATES THE SOCKET AND HANDLES ERRORS
    sock_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd<0){
        perror("socket");
        exit(0);
    }
    
    // BINDS THE SOCKET TO THE PORT
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);

    if(bind(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr))){
        perror("bind");
        exit(0);
    }

    // LISTENS FOR CONNECTIONS
    if(listen(sock_fd, 1)<0){
        perror("listen");
        exit(0);
    }    


    // GET FILE LINES & NUMBER OF CHARS
    get_file_props(&array_holder, filename);


    while (1){
        printf("\nWaiting for connection on 127.0.0.1:%d...\n", port);

        // ACCEPTS THE CONNECTION AND HANDLES ERRORS
        comm_fd=accept(sock_fd, NULL, NULL);
        if(comm_fd<0){
            fprintf(stderr, "Failed to accept the connection:%d\n", comm_fd);
            continue;
        }

        int indx=random_number_gen(0, array_holder.used, general_iterations++);
        randnum=array_holder.array[indx];
        printf("\nrandom number: %d\n\n", randnum);

        iterations=0;
        do{
            // READS THE INPUT FROM THE CLIENT AND HANDLES ERRORS
            if(read(comm_fd, &recv_value, sizeof(recv_value))<0){
                perror("read");
                return 0;
            }
            // CONVERTS FROM NETWORK BYTE ORDER TO HOST BYTE ORDER
            recv_num=(int32_t) ntohl(recv_value);
            printf("\trecieved input: %d\n", recv_num);

            result=compared(randnum, recv_num);
            // CONVERTS FROM HOST BYTE ORDER TO NETWORK BYTE ORDER
            send_value=htonl((uint32_t) result);
            printf("\tsending: %d\n\n", result);

            // SENDS THE RESULT TO THE CLIENT AND HANDLES ERRORS
            if(write(comm_fd, &result, sizeof(result))<0){
                perror("write");
                return 0;
            }

            iterations++;
        }while(result!=0);
        
        printf("Total iterations: %d\n", iterations);
    }

    // CLOSES THE CONNECTION AND THE SOCKET
    close(sock_fd);
    
    // FREE THE ARRAY FROM MEMORY
    free_array(&array_holder);
}