#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>


#define MAX_LINE 100

int compared(int randnum, int num){
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

int main(int argc, char *argv[]){
    if(argc>3){
        printf("Too many arguments\nMaximum 2 argument, you have entered %d arguments\n", argc-1);
        return 0;
    }
    if(argc<2){
        printf("Too few arguments\nMinimum 1 argument, you have entered %d arguments\n", argc-1);
        return 0;
    }
    //printf("Argc: %d\n", argc);

    int PORT=(argc>2)?atoi(argv[2]):9999;
    char *filename=argv[1];

    char textin[MAX_LINE];

    int listen_fd, comm_fd;

    struct sockaddr_in servaddr;

    printf("Server side\n");

    listen_fd=socket(AF_INET, SOCK_STREAM, 0); 

    bzero(&servaddr, sizeof(servaddr)); 

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);

    int numlines=0;
    FILE *file;
    if ((file = fopen(filename,"r")) == NULL){
       printf("Error! opening file: %s", filename);
       return(0);
    }
    printf("file open\n");
    char *myString, c;
    
    //! this method to get the number of lines, it can be done in a better way??
    do{
        c = fgetc(file);
        if(c == '\n' || c == EOF) numlines++;
    } while (c != EOF);
    //printf("\n");
    printf("numlines: %d\n", numlines);

    int length[numlines];
    numlines=0;
    int charcount=0;
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

    /*for(int i=0; i<sizeof(length)/sizeof(int); i++){
        printf("length[%d]: %d\n", i, length[i]);
    } */

    printf("Waiting for connection on 127.0.0.1:%d\n", PORT);


    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listen_fd, 1);

    comm_fd=accept(listen_fd, NULL, NULL);
    
    int randnum, num;

    char textout[5];
    sprintf(textout, "%d", numlines);
    int chrs=strlen(textout);
    write(comm_fd, textout, chrs);
    

    bzero(textin,MAX_LINE);
    read(comm_fd,textin,MAX_LINE);
    num = atoi(textin);
    
    randnum = length[num];

    sprintf(textout, "%d", randnum);
    chrs=strlen(textout);

    write(comm_fd, textout, chrs);
    return(0);
}