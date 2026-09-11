/**
 * @file Task2.c
 * @brief Parallel implementation for finding prime numbers less than n
 * using POSIX threads.
 *
 * This program accepts a positive integer n from the user and determines
 * all prime numbers that are strictly less than n using a parallel algorithm.
 *
 * The program uses dynamic chunk scheduling. Threads repeatedly request
 * chunks of unprocessed numbers, allowing faster threads to take additional
 * work and helping to balance the workload.
 *
 * The number of threads is determined automatically from the number of
 * online processors available on the computer.
 *
 * Prime numbers are output in ascending order. Prime numbers less than
 * 100 are printed to standard output, while larger results are written
 * to a text file.
 *
 * @author Zahra
 * @author Suchir
 * @date 2026
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

/**
 * @brief Number of integers assigned to a thread at a time.
 *
 * Threads dynamically request another chunk after completing their
 * current chunk.
 */
#define CHUNK_SIZE 100000

int n;
int nextNumber;
int *isPrime;

/** Mutex used to protect access to the shared nextNumber counter. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/** Number of threads used by the program. */
int num_threads;

/** Store each thread's CPU time. */
double *threadTimes;

void *ThreadFunc(void *pArg);

/**
 * @brief Main function of the POSIX threads prime-number program.
 *
 * Determines the number of available processors, creates one thread
 * per processor, performs the prime-number computation, and reports
 * computational and overall execution time.
 *
 * @return 0 if execution is successful, otherwise 1.
 */
int main()
{
    struct timespec start, end, startComp, endComp;
    double time_taken;
    char filename[50];

    /*
     * Determine the number of processors available to the program.
     */
    num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);

    if (num_threads < 1) {
        num_threads = 1;
    }

    printf("Available CPU processors: %d\n", num_threads);

    /*
     * Allocate thread IDs, thread numbers and thread timing data
     * according to the number of available processors.
     */
    pthread_t *tid =
        malloc(num_threads * sizeof(pthread_t));

    int *threadNum =
        malloc(num_threads * sizeof(int));

    threadTimes =
        malloc(num_threads * sizeof(double));

    if (tid == NULL ||
        threadNum == NULL ||
        threadTimes == NULL) {

        printf("Memory allocation failed.\n");

        free(tid);
        free(threadNum);
        free(threadTimes);

        return 1;
    }

    printf("Enter the number: ");

    if (scanf("%d", &n) != 1 || n <= 0) {

        printf("Invalid input, please try again.\n");

        free(tid);
        free(threadNum);
        free(threadTimes);

        return 1;
    }

    nextNumber = 2;

    /* Start measuring overall execution time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &start
    );

    snprintf(
        filename,
        sizeof(filename),
        "primes2_%d.txt",
        n
    );

    FILE *file = NULL;

    /* Allocate memory to store prime results. */
    isPrime =
        malloc((size_t)n * sizeof(int));

    if (isPrime == NULL) {

        printf("Memory allocation failed.\n");

        free(tid);
        free(threadNum);
        free(threadTimes);

        return 1;
    }

    /* Create output file for n >= 100. */
    if (n >= 100) {

        file = fopen(filename, "w");

        if (file == NULL) {

            printf("Could not create file.\n");

            free(isPrime);
            free(tid);
            free(threadNum);
            free(threadTimes);

            return 1;
        }
    }

    /* Start measuring computational time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &startComp
    );

    /*
     * Create one POSIX thread for each available processor.
     */
    for (int i = 0; i < num_threads; i++) {

        threadNum[i] = i;

        int result = pthread_create(
            &tid[i],
            NULL,
            ThreadFunc,
            &threadNum[i]
        );

        if (result != 0) {

            printf(
                "Failed to create thread %d.\n",
                i
            );

            free(isPrime);
            free(tid);
            free(threadNum);
            free(threadTimes);

            return 1;
        }
    }

    /* Wait for all threads to finish. */
    for (int i = 0; i < num_threads; i++) {

        pthread_join(
            tid[i],
            NULL
        );
    }

    /* Stop measuring computational time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &endComp
    );

    time_taken =
        (endComp.tv_sec - startComp.tv_sec) * 1e9;

    time_taken =
        (time_taken +
        (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9;

    /*
     * Print individual thread CPU times.
     */
    for (int i = 0; i < num_threads; i++) {

        printf(
            "Thread %d CPU time: %.6f seconds\n",
            i,
            threadTimes[i]
        );
    }

    printf(
        "\nComputational time only(s): %lf\n",
        time_taken
    );

    /*
     * Output prime numbers in ascending order.
     *
     * Although threads process chunks in parallel, the results are
     * stored in isPrime[p]. Therefore, iterating through the array
     * from 2 to n guarantees ascending output order.
     */
    for (int p = 2; p < n; p++) {

        if (isPrime[p] == 1) {

            if (n < 100) {
                printf("%d ", p);
            }
            else {
                fprintf(
                    file,
                    "%d\n",
                    p
                );
            }
        }
    }

    if (n < 100) {

        printf("\n");

    }
    else {

        fclose(file);

        printf(
            "Prime numbers have been written to the text file.\n"
        );
    }

    /* Free prime-number array. */
    free(isPrime);

    /* Stop measuring overall execution time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &end
    );

    time_taken =
        (end.tv_sec - start.tv_sec) * 1e9;

    time_taken =
        (time_taken +
        (end.tv_nsec - start.tv_nsec)) * 1e-9;

    printf(
        "Overall time(s): %lf\n",
        time_taken
    );

    /* Free dynamically allocated thread resources. */
    free(tid);
    free(threadNum);
    free(threadTimes);

    return 0;
}

/**
 * @brief Performs prime-number computation for a single POSIX thread.
 *
 * The thread repeatedly obtains a chunk of unprocessed numbers using
 * the shared nextNumber counter. Access to nextNumber is synchronised
 * using a mutex.
 *
 * Each number is tested using an optimised primality test. Even numbers
 * greater than 2 are skipped, and only odd divisors up to sqrt(p)
 * are examined.
 *
 * @param pArg Pointer to the thread's integer identifier.
 *
 * @return NULL after the thread has completed all available work.
 */
void *ThreadFunc(void *pArg)
{
    int my_rank = *((int *)pArg);

    struct timespec threadStart, threadEnd;
    double threadTime;

    clock_gettime(
        CLOCK_THREAD_CPUTIME_ID,
        &threadStart
    );

    while (1) {

        /* Get the next chunk of work. */
        pthread_mutex_lock(&mutex);

        int start = nextNumber;
        int end = start + CHUNK_SIZE - 1;

        nextNumber = end + 1;

        pthread_mutex_unlock(&mutex);

        /* No more numbers to process. */
        if (start >= n) {
            break;
        }

        /* Do not process beyond n - 1. */
        if (end >= n) {
            end = n - 1;
        }

        /* Process every number in the assigned chunk. */
        for (int p = start; p <= end; p++) {

            /* Assume p is not prime. */
            isPrime[p] = 0;

            /* 2 is prime. */
            if (p == 2) {
                isPrime[p] = 1;
                continue;
            }

            /* Even numbers greater than 2 are not prime. */
            if (p % 2 == 0) {
                continue;
            }

            bool prime = true;

            /* Only check divisors up to sqrt(p). */
            int limit = (int)sqrt((double)p);

            /* Check odd divisors. */
            for (int i = 3; i <= limit; i += 2) {

                if (p % i == 0) {
                    prime = false;
                    break;
                }
            }

            /* No divisor found, therefore p is prime. */
            if (prime) {
                isPrime[p] = 1;
            }
        }
    }

    /* Measure CPU time used by this thread. */
    clock_gettime(
        CLOCK_THREAD_CPUTIME_ID,
        &threadEnd
    );

    threadTime =
        (threadEnd.tv_sec - threadStart.tv_sec) * 1e9;

    threadTime =
        (threadTime +
        (threadEnd.tv_nsec - threadStart.tv_nsec)) * 1e-9;

    threadTimes[my_rank] = threadTime;

    return NULL;
}