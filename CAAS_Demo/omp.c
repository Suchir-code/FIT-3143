/* Header files */
#include<stdio.h>
#include<stdlib.h>
#include<omp.h>

int main(int argc, char* argv[]){

	/*
	int NUM_THREAD = 4;
	omp_set_num_threads(NUM_THREAD);
	*/
	
	#pragma omp parallel
	{
		int thread_number = omp_get_thread_num();
		
		printf("Hello World from thread %d\n", thread_number);
	}

	return 0;

}
