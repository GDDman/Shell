#include <pthread.h>
#include <stdlib.h>

int maxes[5];
int nums[5000]; 

int main(void) {

	int i;
	for (i=0; i < 5000; i++) {
		nums[i] = rand() % 1001;
	}

	int j;
	for (j=0; j < 5; j++) {

		int *temp = malloc(1000 * sizeof(int));
		pthread_create(void, NULL, threadFunc, temp + (1000 * j), j);
	}

	int min = 0;
	int p;
	for (p=0; p < 5; p++) {
		if (maxes[p] < min) min = maxes[p];
		printf("\n%d", maxes[p]);
	}	
	printf("MIN %d", min);
}

void threadFunc(* temp, int pos) {
	
	int min = 0;
	int i;
	for (i = 0; i < 1000; i++) {
		if (temp[i] < min) min = temp[i];
	}
	maxes[pos] = min;	

}
