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
    if(argc>3){
        printf("Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        return(0);
    }if(argc<2){
        printf("Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
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
        port=(argc>2)?atoi(argv[2]):DEFAULT_PORT,
        listen_fd,
        comm_fd,
        totaliterations=0,
        numlines=0,
        charcount=0,
        randnum=0;

    char 
        *filename=argv[1],
        *myString,
        c;

    //char textin[MAX_LINE]; //? can i delet this??

    struct sockaddr_in servaddr;

    FILE *file;

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

    if ((file = fopen(filename,"r")) == NULL){
       printf("Error! opening file: %s", filename);
       return(0);
    }
    printf("file open\n");
    
    //! this method get the number of lines, it can be done in a better way??
    do{
        c = fgetc(file);
        if(c == '\n' || c == EOF) numlines++;
    } while (c != EOF);

    printf("numlines: %d\n", numlines);

    int length[numlines];
    numlines=0;

    rewind(file);

    do{
        c = fgetc(file);
        charcount++;
        if(c == '\n' || c == EOF){  
            length[numlines]=charcount%100;
            numlines++;
            charcount=0;
        }
    } while (c != EOF);    
    
    printf("file read\n");
    fclose(file);

    printf("Waiting for connection on 127.0.0.1:%d\n", port);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listen_fd, 1);

    
    while (1){
        comm_fd=accept(listen_fd, NULL, NULL);

        randnum = length[random_number_gen(0, sizeof(length)/sizeof(int), totaliterations)];
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
        }while(result!='0');
        
        printf("Total iterations: %d\n", totaliterations);
    }
    close(listen_fd);
}