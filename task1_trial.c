#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    int n;
    int capacity = 3;
    int count = 0;
    int *primeList = (int*)malloc(capacity * sizeof(int));
    struct timespec start, end, startComp, endComp; 
    double time_taken;

    // Get current clock time.
	clock_gettime(CLOCK_MONOTONIC, &start); 

    printf("Please enter a number: ");
    scanf("%d", &n);

    // Get current clock time.
	clock_gettime(CLOCK_MONOTONIC, &startComp); 

    for (int i = 2; i < n; i++){
        int cnt = 0;

        // If number is less than/equal 
        // to 1 and number is even accept 2
        // then it is not prime
        if ((i > 2) && (i%2 == 0))
            printf("%d is NOT prime\n", i);
        else {

            if(i==2){
                printf("%d is prime\n", i);
                if (count == capacity) {
                    capacity *= 2;
                    primeList = realloc(primeList, capacity * sizeof(int));
                }

                primeList[count] = i;
                count++;
            }else{
                
            // Check how many numbers divide n in
            // range 2 to sqrt(n)
            for (int j = 3; j <= sqrt(i); j+=2) {
                if (i % j == 0)
                    cnt++;
            }

            // if cnt is greater than 0 then n is
            // not prime
            if (cnt > 0)
                printf("%d is NOT prime\n", i);

            // else n is prime
            else {
                printf("%d is prime\n", i);

                if (count == capacity) {
                    capacity *= 2;
                    primeList = realloc(primeList, capacity * sizeof(int));
                }

                primeList[count] = i;
                count++;
            }
            }
        }
    }

    // Get the clock current time again
	// Subtract end from start to get the CPU time used.
	clock_gettime(CLOCK_MONOTONIC, &endComp); 
	time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9; 
    	time_taken = (time_taken + (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9; 
	printf("Cell product complete - Computational time only(s): %lf\n", time_taken); // portion of the computing time of ts


    // Print the prime list
    printf("\nPrime list:\n");

    for (int i = 0; i < count; i++) {
        printf("%d ", primeList[i]);
    }

    printf("\n");

    free(primeList);

    // Get the clock current time again
	// Subtract end from start to get the CPU time used.
	clock_gettime(CLOCK_MONOTONIC, &end); 
	time_taken = (end.tv_sec - start.tv_sec) * 1e9; 
    	time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9; 
	printf("Overall time (Including read, product and write)(s): %lf\n", time_taken);	// ts
	

    return 0;
}