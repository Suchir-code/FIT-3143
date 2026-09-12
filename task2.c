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
int next_number;
int *is_prime;

/** Mutex used to protect access to the shared next_number counter. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/** Number of threads used by the program. */
int num_threads;

/** Stores each thread's CPU time. */
double *thread_times;

/** POSIX worker-thread function. */
void *thread_func(void *arg);

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
    struct timespec start, end, start_comp, end_comp;
    double time_taken;
    char file_name[50];

    /*
     * Determine the number of processors available to the program.
     */
    num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);

    if (num_threads < 1) {
        num_threads = 1;
    }

    printf(
        "Available CPU processors: %d\n",
        num_threads
    );

    /*
     * Allocate thread IDs, thread numbers and thread timing data
     * according to the number of available processors.
     */
    pthread_t *thread_ids =
        malloc(num_threads * sizeof(pthread_t));

    int *thread_numbers =
        malloc(num_threads * sizeof(int));

    thread_times =
        malloc(num_threads * sizeof(double));

    if (thread_ids == NULL ||
        thread_numbers == NULL ||
        thread_times == NULL) {

        printf("Memory allocation failed.\n");

        free(thread_ids);
        free(thread_numbers);
        free(thread_times);

        return 1;
    }

    printf("Enter the number: ");

    if (scanf("%d", &n) != 1 || n <= 0) {

        printf("Invalid input, please try again.\n");

        free(thread_ids);
        free(thread_numbers);
        free(thread_times);

        return 1;
    }

    next_number = 2;

    /* Start measuring overall execution time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &start
    );

    snprintf(
        file_name,
        sizeof(file_name),
        "primes2_%d.txt",
        n
    );

    FILE *file = NULL;

    /* Allocate memory to store prime results. */
    is_prime =
        malloc((size_t)n * sizeof(int));

    if (is_prime == NULL) {

        printf("Memory allocation failed.\n");

        free(thread_ids);
        free(thread_numbers);
        free(thread_times);

        return 1;
    }

    /* Create output file for n >= 100. */
    if (n >= 100) {

        file = fopen(file_name, "w");

        if (file == NULL) {

            printf("Could not create file.\n");

            free(is_prime);
            free(thread_ids);
            free(thread_numbers);
            free(thread_times);

            return 1;
        }
    }

    /* Start measuring computational time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &start_comp
    );

    /*
     * Create one POSIX thread for each available processor.
     */
    for (int i = 0; i < num_threads; i++) {

        thread_numbers[i] = i;

        int result = pthread_create(
            &thread_ids[i],
            NULL,
            thread_func,
            &thread_numbers[i]
        );

        if (result != 0) {

            printf(
                "Failed to create thread %d.\n",
                i
            );

            free(is_prime);
            free(thread_ids);
            free(thread_numbers);
            free(thread_times);

            return 1;
        }
    }

    /* Wait for all threads to finish. */
    for (int i = 0; i < num_threads; i++) {

        pthread_join(
            thread_ids[i],
            NULL
        );
    }

    /* Stop measuring computational time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &end_comp
    );

    time_taken =
        (end_comp.tv_sec - start_comp.tv_sec) * 1e9;

    time_taken =
        (time_taken +
        (end_comp.tv_nsec - start_comp.tv_nsec)) * 1e-9;

    /*
     * Print individual thread CPU times.
     */
    for (int i = 0; i < num_threads; i++) {

        printf(
            "Thread %d CPU time: %.6f seconds\n",
            i,
            thread_times[i]
        );
    }

    printf(
        "\nComputational time only(s): %lf\n",
        time_taken
    );

    /*
     * Output prime numbers in ascending order.
     *
     * Threads only compute and store results in is_prime[p].
     * File output is performed serially after all threads finish.
     * This avoids concurrent writes to the same file and guarantees
     * that prime numbers are written in ascending order.
     */
    for (int p = 2; p < n; p++) {

        if (is_prime[p] == 1) {

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
    free(is_prime);

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
    free(thread_ids);
    free(thread_numbers);
    free(thread_times);

    return 0;
}

/**
 * @brief Performs prime-number computation for a single POSIX thread.
 *
 * Each thread repeatedly requests the next available chunk of numbers.
 * Access to the shared next_number variable is protected by a mutex.
 *
 * Prime computation takes place outside the critical section so threads
 * can perform the expensive computation concurrently.
 *
 * Dynamic chunk allocation improves load balancing because candidate
 * numbers can require different amounts of computation. A thread that
 * finishes one chunk can request another instead of remaining idle.
 *
 * @param arg Pointer to the thread's integer identifier.
 *
 * @return NULL after the thread has completed all available work.
 */
void *thread_func(void *arg)
{
    int thread_id = *((int *)arg);

    struct timespec thread_start, thread_end;
    double thread_time;

    clock_gettime(
        CLOCK_THREAD_CPUTIME_ID,
        &thread_start
    );

    while (1) {

        /*
         * Obtain the next available chunk.
         * Only chunk allocation is protected by the mutex.
         */
        pthread_mutex_lock(&mutex);

        int start = next_number;
        int end = start + CHUNK_SIZE - 1;

        next_number = end + 1;

        pthread_mutex_unlock(&mutex);

        /* No more numbers remain to process. */
        if (start >= n) {
            break;
        }

        /* Do not process beyond n - 1. */
        if (end >= n) {
            end = n - 1;
        }

        /*
         * Prime checking occurs outside the critical section,
         * allowing multiple threads to compute simultaneously.
         */
        for (int p = start; p <= end; p++) {

            /* Initially assume p is not prime. */
            is_prime[p] = 0;

            /* 2 is prime. */
            if (p == 2) {
                is_prime[p] = 1;
                continue;
            }

            /* Even numbers greater than 2 are not prime. */
            if (p % 2 == 0) {
                continue;
            }

            bool prime = true;

            /* Only divisors up to sqrt(p) need to be checked. */
            int limit = (int)sqrt((double)p);

            /* Check only odd divisors. */
            for (int i = 3; i <= limit; i += 2) {

                if (p % i == 0) {
                    prime = false;
                    break;
                }
            }

            /* No divisor found, therefore p is prime. */
            if (prime) {
                is_prime[p] = 1;
            }
        }
    }

    /* Measure CPU time used by this thread. */
    clock_gettime(
        CLOCK_THREAD_CPUTIME_ID,
        &thread_end
    );

    thread_time =
        (thread_end.tv_sec - thread_start.tv_sec) * 1e9;

    thread_time =
        (thread_time +
        (thread_end.tv_nsec - thread_start.tv_nsec)) * 1e-9;

    thread_times[thread_id] = thread_time;

    return NULL;
}