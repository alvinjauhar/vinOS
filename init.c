
#include "types.h"
#include "user.h"
#include "x86.h"

int main(void){

	int pid;
	pid = fork();
	if (pid == 0){
		for (int i = 0; i < 4; i++){
			pid = fork();
			if (pid == 0){
				printf("child %d created \n", getpid());
				exit();
			} else if (pid > 0){
				printf("parent %d creating child %d \n", getpid(), pid);
				pid = wait();
				printf("child %d done\n", pid);
			}
		}
	} else if (pid > 0) {
		for (;;)
		pause();
	}

	for(;;);
}

/*
int main(void){

	int pid;

	pid = fork();
//	printf("after fork in init\n");
	for(;;);
	if (pid > 0){
//		printf("parent\n");
		wait();	
	} else if (pid == 0) {
		printf("child\n");
	}

	for(;;);
}
*/
