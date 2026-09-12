/* Header files */
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

/* Function Declaration */
void *threadFunc(void *arg);

int main(int argc, char* argv[]){

	int NUM_THREAD = 4;
	pthread_t hThread[NUM_THREAD]; 
	int threadNum[NUM_THREAD];
	
	int i;
	
	// Create Thread
	for(i=0; i<NUM_THREAD; i++){
		threadNum[i] = i;
		pthread_create(&hThread[i], NULL, threadFunc, &threadNum[i]);
	}
	
	// Join Thread
	for(i=0; i<NUM_THREAD; i++){
		pthread_join(hThread[i], NULL);
	}

	return 0;

}

void *threadFunc(void *arg){
	int thread_number = *(int*) arg;
	printf("Hello World from thread %d\n", thread_number);
}
