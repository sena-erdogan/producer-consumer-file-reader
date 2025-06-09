#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h>

#define READ_FLAGS O_RDONLY

int C;
int N;
int FD;
int cn;

int bytesread = 0;
	
char buf[1];

union semun  
{
        int val;
};

union semun sem1; //Semaphore for '1'
union semun sem2; //Semaphore for '2'

void* consumer(){
	int i;
	time_t ct = time(NULL);
	struct tm *tm;
	for(i=0; i<N; i++){
		if(sem1.val>0 && sem2.val>0){
			tm = localtime(&ct);
			printf("%s Consumer-%d at iteration %d (waiting). Current amounts: %d x '1', %d x '2'.\n", asctime(tm), cn, i, sem1.val, sem2.val);
			sem1.val--;
			sem2.val--;
			tm = localtime(&ct);
			printf("%s Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x '1', %d x '2'.\n", asctime(tm), cn, i, sem1.val, sem2.val);
		}
	}
	
	tm = localtime(&ct);
	printf("%s Consumer-%d has left.\n", asctime(tm), cn);
	return 0;
}

void* supplier(){
	time_t ct = time(NULL);
	struct tm *tm;
	for( ; ; ){
		while(((bytesread = read(FD, buf, 1)) == -1) && (errno == EINTR));
		
		if(bytesread <= 0)	break;
		
		if(buf[0] == '1'){
			tm = localtime(&ct);
			printf("%s Supplier: read from input a '1'. Current amounts: %d x '1', %d x '2'.\n", asctime(tm), sem1.val, sem2.val);
			sem1.val++;
		}else if(buf[0] == '2'){
			tm = localtime(&ct);
			printf("%s Supplier: read from input a '2'. Current amounts: %d x '1', %d x '2'.\n", asctime(tm), sem1.val, sem2.val);
			sem2.val++;
		}
		
		consumer();
		
		tm = localtime(&ct);
		
		if(buf[0] == '1')	printf("%s Supplier: delivered a '1'. Post-delivery amounts: %d x '1', %d x '2'.\n", asctime(tm), sem1.val, sem2.val);
		else if(buf[0] == '2')	printf("%s Supplier: delivered a '2'. Post-delivery amounts: %d x '1', %d x '2'.\n", asctime(tm), sem1.val, sem2.val);
	}
	printf("The supplier has left\n");
	return 0;
}

int main(int argc, char *argv[]){
	
	int i;
	
	if(argc != 7){
		perror("\nUsage: Threads and System V Semaphores\nformat: ./hw4 -C 10 -N 5 -F inputfilePath\n");
		return 1;
	}
	
	for(i=1; i<=5; i+=2){
		if(strcmp(argv[i], "-F") == 0){
			if((FD = open(argv[i+1], READ_FLAGS)) == -1){
				perror("Failed to open input file\n");
				return 1;
			}
		}else if(strcmp(argv[i], "-C") == 0){
			C = atoi(argv[i+1]);
			if(C<=4){
				perror("C should be larger than 4");
				exit(1);
			}
		}else if(strcmp(argv[i], "-N") == 0){
			N = atoi(argv[i+1]);
			if(N<=1){
				perror("N should be larger than 1");
				exit(1);
			}
		}else{
			perror("\nInvalid command\nformat: ./hw3named -i inputFilePath -n name\n");
			return 1;
		}
	}
	
	sem1.val = 0;
	sem2.val = 0;
	
	srand(time(NULL));
	pthread_t th[C];
	pthread_t sup;
	for (i = -1; i < C; i++){
		cn = i;
		if (i == -1) {
			if (pthread_create(&sup, NULL, &supplier, NULL) != 0) {
				perror("Failed to create supplier thread");
			}
		}else{
			if (pthread_create(&th[i], NULL, &consumer, NULL) != 0) {
				perror("Failed to create consumer thread");
			}
		}
	}
	if (pthread_detach(sup) != 0) {
		perror("Failed to detach consumer thread");
	}
	for (i = 0; i < C; i++){
		if (pthread_join(th[i], NULL) != 0)	perror("Failed to join thread");
	}
		
	if(close(FD) == -1){
		perror("Failed to close input file\n");
		return 1;
	}
	
	return 0;
}
