#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 10
#define CHUNK_SIZE 100000

int n;
int nextNumber;

int *isPrime;

// Store each thread's CPU time
double threadTimes[NUM_THREADS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototype
void *ThreadFunc(void *pArg);

int main() {
    struct timespec start, end, startComp, endComp;
    double time_taken;
    char filename[50];
    int i;

    pthread_t tid[NUM_THREADS];
    int threadNum[NUM_THREADS];

    printf("Enter the number: ");

    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input, please try again.\n");
        return 1;
    }

    nextNumber = 2;

    // Start measuring overall execution time
    clock_gettime(CLOCK_MONOTONIC, &start);

    snprintf(filename, sizeof(filename), "primes2_%d.txt", n);

    FILE *file = NULL;

    // Allocate memory to store prime results
    isPrime = malloc((size_t)n * sizeof(int));

    if (isPrime == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // For n greater than or equal to 100, create a file
    if (n >= 100) {
        file = fopen(filename, "w");

        if (file == NULL) {
            printf("Could not create file.\n");
            free(isPrime);
            return 1;
        }
    }

    // Start measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &startComp);

    // Create threads
    for (i = 0; i < NUM_THREADS; i++) {
        threadNum[i] = i;

        pthread_create(
            &tid[i],
            NULL,
            ThreadFunc,
            &threadNum[i]
        );
    }

    // Wait for all threads to finish
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    // Stop measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &endComp);

    time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9;
    time_taken = (time_taken +
                  (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9;


    // Print individual thread CPU times AFTER computational timing
    for (i = 0; i < NUM_THREADS; i++) {
        printf("Thread %d CPU time: %.6f seconds\n",
               i, threadTimes[i]);
    }

    printf("\nComputational time only(s): %lf\n", time_taken);


    // Output primes AFTER computational timing
    for (int p = 2; p < n; p++) {

        if (isPrime[p] == 1) {

            if (n < 100) {
                printf("%d ", p);
            } else {
                fprintf(file, "%d\n", p);
            }
        }
    }

    if (n < 100) {
        printf("\n");
    } else {
        fclose(file);
        printf("Prime numbers have been written to the text file.\n");
    }

    // Free allocated memory
    free(isPrime);

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

    // Individual thread CPU timing
    struct timespec threadStart, threadEnd;
    double threadTime;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &threadStart);

    while (1) {

        // Get the next chunk of work
        pthread_mutex_lock(&mutex);

        int start = nextNumber;
        int end = start + CHUNK_SIZE - 1;

        nextNumber = end + 1;

        pthread_mutex_unlock(&mutex);

        // No more numbers to process
        if (start >= n) {
            break;
        }

        // Do not go beyond n - 1
        if (end >= n) {
            end = n - 1;
        }

        // Process this chunk
        for (int p = start; p <= end; p++) {

            // Start by assuming p is not prime
            isPrime[p] = 0;

            // 2 is prime
            if (p == 2) {
                isPrime[p] = 1;
                continue;
            }

            // Even numbers greater than 2 are not prime
            if (p % 2 == 0) {
                continue;
            }

            bool prime = true;

            // Calculate sqrt only once for each number
            int limit = (int)sqrt((double)p);

            // Check odd divisors from 3 to sqrt(p)
            for (int i = 3; i <= limit; i += 2) {

                if (p % i == 0) {
                    prime = false;
                    break;
                }
            }

            // If no divisor was found, p is prime
            if (prime) {
                isPrime[p] = 1;
            }
        }
    }

    // Stop individual thread CPU timing
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &threadEnd);

    threadTime = (threadEnd.tv_sec - threadStart.tv_sec) * 1e9;
    threadTime = (threadTime +
                  (threadEnd.tv_nsec - threadStart.tv_nsec)) * 1e-9;

    threadTimes[my_rank] = threadTime;

    return NULL;
}
