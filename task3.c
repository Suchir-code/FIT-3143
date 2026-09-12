/**
 * @file Task3.c
 * @brief Parallel implementation for finding prime numbers less than n
 * using OpenMP.
 *
 * This program accepts a positive integer n from the user and determines
 * all prime numbers that are strictly less than n using OpenMP parallelisation.
 *
 * The prime-number computation is distributed among OpenMP threads using
 * dynamic scheduling with a fixed chunk size. Dynamic scheduling allows
 * threads that finish their assigned chunks earlier to obtain additional
 * chunks of work, helping to balance the workload.
 *
 * Prime numbers are output in ascending order. Prime numbers less than
 * 100 are printed to standard output, while larger results are written
 * to a text file.
 *
 * @author Suchir
 * @author Zahra
 * @date 2026
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define CHUNK_SIZE 10000

/**
 * @brief Main entry point of the OpenMP prime-number program.
 *
 * Reads the input value n, allocates memory for storing prime results,
 * performs the parallel prime-number computation, measures execution time,
 * outputs the results, and releases allocated memory.
 *
 * @return 0 if the program completes successfully, or 1 if an error occurs.
 */
int main()
{
    int n;
    struct timespec start, end, start_comp, end_comp;
    double time_taken;
    char file_name[50];

    printf("Enter the number: ");

    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input, please try again.\n");
        return 1;
    }

    /* Start measuring overall execution time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &start
    );

    snprintf(
        file_name,
        sizeof(file_name),
        "primes3_%d.txt",
        n
    );

    FILE *file = NULL;

    /* Allocate memory to store prime results. */
    int *is_prime =
        malloc((size_t)n * sizeof(int));

    if (is_prime == NULL) {

        printf("Memory allocation failed.\n");

        return 1;
    }

    /* Create output file for n >= 100. */
    if (n >= 100) {

        file = fopen(file_name, "w");

        if (file == NULL) {

            printf("Could not create file.\n");

            free(is_prime);

            return 1;
        }
    }

    /* Start measuring computational time. */
    clock_gettime(
        CLOCK_MONOTONIC,
        &start_comp
    );

    /**
     * Parallel prime-number computation.
     *
     * The loop iterations are distributed among OpenMP threads using
     * dynamic scheduling. Each thread receives CHUNK_SIZE candidate
     * numbers at a time.
     *
     * Dynamic scheduling is used because prime checking does not always
     * require the same amount of work for every candidate. Threads that
     * finish their current chunk can receive another available chunk,
     * reducing the chance of threads remaining idle.
     *
     * Each iteration writes only to its own is_prime[p] location.
     * Therefore, no mutex or OpenMP critical section is required for
     * storing the prime result.
     */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE)
    for (int p = 2; p < n; p++) {

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

        /*
         * A composite number must have a divisor less than or equal
         * to its square root, so no larger divisors need to be checked.
         */
        int limit = (int)sqrt((double)p);

        /* Check only odd divisors from 3 to sqrt(p). */
        for (int i = 3; i <= limit; i += 2) {

            if (p % i == 0) {
                prime = false;
                break;
            }
        }

        /* If no divisor was found, p is prime. */
        if (prime) {
            is_prime[p] = 1;
        }
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

    printf(
        "\nComputational time only(s): %lf\n",
        time_taken
    );

    /*
     * File output is performed after the parallel region.
     *
     * Writing results serially avoids concurrent access to the same
     * output file. Scanning is_prime from 2 to n also guarantees that
     * the prime numbers are written in ascending order.
     *
     * Output is kept outside computational timing so file I/O does not
     * affect the measured prime-computation performance.
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

    /* Free allocated memory. */
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

    return 0;
}