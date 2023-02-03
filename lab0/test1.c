#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
  time_t t;
  long long int inici = time(&t);
  printf("Start time: %ld seconds\n", time(&t));
  printf("%lld\n", inici);
  if(argc>2){
    printf("Too many arguments\nMaximum 1 argument, you have entered %d arguments\n", argc-1);
    return 0;
  }
  
  int pmin=0, pmax=(argc<2)?100:atoi(argv[1]), min, max;
  printf("Max: %d\n",pmax);
  
  int randnum, num, j;
  int totaliterations=0;

  for(int i=0; i<10; i++) {
    min=pmin;
    max=pmax;
    randnum = random_number_gen(min, max, totaliterations);
    printf("%d. Random number: %d\n", i, randnum);

    num=max/2;
    for(j=0; j<10; j++) { //? do we have a limit of iterations??
      printf("\n\n%d.%d. Num: %d\n", i, j , num);

      int result = compared(randnum, num);
      if(max-min==1){
        printf("Random number is the max\n");
        num = max;
      }
      else if(result==0){
        printf("\tRandom number is equal to %d\n", num);
        break;
      }
      else if(result==1){
        printf("\tRandom number is less than %d\n", num);
        max = num;
        num = (max+min)/2;
      }
      else{
        printf("\tRandom number is greater than %d\n", num);
        min = num;
        num = (max+min)/2;
      }

      printf("\tMax: %d\n\tMin: %d\n", max, min);
    }

    printf(j==10?"\n\nIm sorry":"\n\nCongratulations");
    printf(", random number is %d\n", num);

    printf("Iterations: %d\n", j);
    totaliterations += j;

    printf("\nPress enter to continue...");
    // getchar();
    printf("\n\n");
  }
  printf("Average iterations: %d\n", totaliterations/10);

  long long int final = time(&t);
  printf("End time: %ld seconds\n", time(&t));
  printf("%lld\n", final);

  printf("Execution time: %lld seconds\n", final-inici);

  return 0;
}