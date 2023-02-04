#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h> // inet_pton()
#include <unistd.h> // read, write, close

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
    if(argc>2){
        printf("Too many arguments\nMaximum 1 argument, you have entered %d arguments\n", argc-1);
        return 0;
    }
    printf("Argc: %d\n", argc);

    int PORT=(argc==2)?atoi(argv[1]):8080;

    char textin[MAX_LINE]; // variable where the incoming text will be saved.

    int listen_fd, comm_fd; // file descriptors to be used

    struct sockaddr_in servaddr; // struct to hold ips and ports,
    // https://vdc-repo.vmware.com/vmwb-repository/dcr-public/c509579b-fc98-4ec2-bf0c-cadaebc51017/f572d815-0e80-4448-a354-dff39a1d545e/doc/vsockAppendix.8.3.html

    printf("Server side\n");

    listen_fd=socket(AF_INET, SOCK_STREAM, 0); // creates a listen connection
    // creates a socket with AF_INET (ip family) and SOCK_STREAM
    // "Data from all devices wishing to connect on this socket will be redirected to listen_fd."

    // sets all servaddr bytes to 0; 
    bzero(&servaddr, sizeof(servaddr)); 

    servaddr.sin_family=AF_INET; // set addressing scheme
    servaddr.sin_port=htons(PORT); // port number
    servaddr.sin_addr.s_addr=htons(INADDR_ANY); // allows to connect any ip

    printf("Waiting for connection on port %d\n" , PORT);

    // prepare to listen for connections from ip/port specified earlier
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // start listening. maximum N connections (1 in this case), 
    listen(listen_fd, 1);

    // accept connection comming from listen file descriptor, if theres noone wait.
    comm_fd=accept(listen_fd, NULL, NULL);
    // read comm_fd = sent by the client
    // write comm_fd = will be read by the client.

    int totaliterations=0, min=0, max=100, num, result;
    
    int randnum = random_number_gen(min, max, totaliterations);
    printf("Random number: %d\n\n", randnum);

    do{
        bzero(textin,MAX_LINE);

        //int to string conversion
        // int chrs=read(comm_fd,textin,MAX_LINE);
        read(comm_fd,textin,MAX_LINE);
        num = atoi(textin);
        printf("Text input: %d\n", num);

        result = compared(randnum, num);
        char textout[5];
        sprintf(textout, "%d", result);
        int chrs=strlen(textout);
        printf("Text output: %s\n", textout);
        printf("\n\n");

        write(comm_fd, textout, chrs);
        totaliterations++;
    }while(result!='0');
}