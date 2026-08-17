#include <math.h> 
#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <pthread.h>

#define NUM_THREADS 8

int numbers_to_check;
int numbers_per_thread;
int n; 



int *primeLists[NUM_THREADS];
int primeCounts[NUM_THREADS];

// Funciton prototype
void *ThreadFunc(void *pArg); // POSIX thread function format
 
int main() { 
    int cnt = 0; 
    struct timespec start, end, startComp, endComp; 
    double time_taken; 
    char filename[50];
    int i;

    pthread_t tid[NUM_THREADS];
    int threadNum[NUM_THREADS];
 

    printf("Enter the number: "); 

    // Start measuring overall execution time
 
    if (scanf("%d", &n) != 1 || n <= 0) { 
        printf("Invalid input, please try again.\n"); 
        return 1; 
    } 

    clock_gettime(CLOCK_MONOTONIC, &start); 

    snprintf(filename, sizeof(filename), "primes2_%d.txt", n);

    FILE *file = NULL; 
 
    // For n greater than 100 we create a file 
    if (n >= 100) { 
        file = fopen(filename, "w"); 
 
        if (file == NULL) { 
            printf("Could not create file.\n"); 
            return 1; 
        } 
    } 

    // Allocate space for each thread's results
    for (int i = 0; i < NUM_THREADS; i++) {
        primeLists[i] = malloc(n * sizeof(int));
        primeCounts[i] = 0;
    }
 
    // Start measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &startComp); 
 
    for (i = 0; i < NUM_THREADS; i++){
        threadNum[i] = i;
        pthread_create(&tid[i], 0, ThreadFunc, &threadNum[i]);
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(tid[i], NULL);
    }
 
    // Stop measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &endComp); 
 
    time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9; 
    time_taken = (time_taken + 
                 (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9; 
 
    printf("\nComputational time only(s): %lf\n", time_taken); 

    /*
       Print results in thread/range order.
       Since each thread handles a consecutive range,
       this keeps the primes sorted.
    */
    for (int i = 0; i < NUM_THREADS; i++) {

        for (int j = 0; j < primeCounts[i]; j++) {

            if (n < 100) {
                printf("%d ", primeLists[i][j]);
            } else {
                fprintf(file, "%d\n", primeLists[i][j]);
            }
        }
    }
 
    if (n < 100) { 
        printf("\n"); 
    } else { 
        fclose(file); 
        printf("Prime numbers have been written to the text file.\n"); 
    } 

    // Free each thread's list
    for (int i = 0; i < NUM_THREADS; i++) {
        free(primeLists[i]);
    }
 
    // Stop measuring overall execution time
    clock_gettime(CLOCK_MONOTONIC, &end); 
 
    time_taken = (end.tv_sec - start.tv_sec) * 1e9; 
    time_taken = (time_taken + 
                 (end.tv_nsec - start.tv_nsec)) * 1e-9; 
 
    printf("Overall time(s): %lf\n", time_taken); 
 
    return 0; 
}

// Thread function
void *ThreadFunc(void *pArg)
{
    int my_rank = *((int *)pArg);

    int numbers_to_check = n - 2;
    int numbers_per_thread = numbers_to_check / NUM_THREADS;
    int remainder = numbers_to_check % NUM_THREADS;

    int start = 2 + my_rank * numbers_per_thread;
    int end = start + numbers_per_thread - 1;

    // Give remaining numbers to the last thread
    if (my_rank == NUM_THREADS - 1) {
        end += remainder;
    }

    printf("Thread %d: checking %d to %d\n",
           my_rank, start, end);

    for (int p = start; p <= end; p++) {

        int cnt = 0;

        // 2 is prime
        if (p == 2) {
            primeLists[my_rank][primeCounts[my_rank]] = p;
            primeCounts[my_rank]++;
            continue;
        }

        // Even numbers greater than 2 are not prime
        if (p % 2 == 0) {
            continue;
        }

        // Check odd divisors from 3 to sqrt(p)
        for (int i = 3; i <= sqrt(p); i += 2) {

            if (p % i == 0) {
                cnt++;
                break;
            }
        }

        // No divisor found → prime
        if (cnt == 0) {
            primeLists[my_rank][primeCounts[my_rank]] = p;
            primeCounts[my_rank]++;
        }
    }

    return NULL;
}