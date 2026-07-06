
#include "types.h"
#include "user.h"
#include "x86.h"

int main(void){

	printf("init starting!\n");

	int pid;

	pid = fork();
	if (pid > 0){
		printf("parent %d creating child %d \n", getpid(), pid);
	} else if (pid == 0){
		printf("child %d created \n", getpid());
	} else {
		printf("fork error\n");
	}

	for (;;);
}
