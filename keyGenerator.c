#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN 00000000
#define MAX 100000000

int main(int argc, char *argv[]){
	int numberOfKeys;
	int i;

	if(argv[1] == NULL){
		printf("Introduce the number of keys that you want generate\n");
	}else{
		numberOfKeys = atoi(argv[1]);
	}

	double key[numberOfKeys];
	
	srand(time(NULL));

	for(i = 0; i < numberOfKeys; i++){
		key[i] = MIN + rand() % (MAX - MIN);
		printf("%.0f \n", key[i]);
	}
}