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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define CHUNK_SIZE 10000

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

    int limit;

    if (n > 2)
        limit = (int)sqrt((double)(n - 1));
    else
        limit = 1;

    /*
     * Generate base primes up to sqrt(n).
     */
    unsigned char *base_sieve =
        malloc((size_t)(limit + 1) * sizeof(unsigned char));

    if (base_sieve == NULL) {
        printf("Memory allocation failed.\n");

        if (file != NULL)
            fclose(file);

        free(is_prime);
        return 1;
    }

    for (int i = 0; i <= limit; i++)
        base_sieve[i] = 1;

    if (limit >= 0)
        base_sieve[0] = 0;

    if (limit >= 1)
        base_sieve[1] = 0;

    for (int prime = 2; prime * prime <= limit; prime++) {
        if (base_sieve[prime] == 1) {
            for (int multiple = prime * prime;
                 multiple <= limit;
                 multiple += prime) {

                base_sieve[multiple] = 0;
            }
        }
    }

    int base_count = 0;

    for (int i = 2; i <= limit; i++) {
        if (base_sieve[i] == 1)
            base_count++;
    }

    int *base_primes = NULL;

    if (base_count > 0) {
        base_primes =
            malloc((size_t)base_count * sizeof(int));

        if (base_primes == NULL) {
            printf("Memory allocation failed.\n");

            free(base_sieve);

            if (file != NULL)
                fclose(file);

            free(is_prime);
            return 1;
        }

        int index = 0;

        for (int i = 2; i <= limit; i++) {
            if (base_sieve[i] == 1) {
                base_primes[index] = i;
                index++;
            }
        }
    }

    free(base_sieve);

    /*
     * Parallel Sieve of Eratosthenes.
     *
     * The range is divided into chunks. OpenMP dynamically distributes
     * the chunks between threads. Each thread works only on the section
     * assigned to it, so no critical section is required.
     */
    #pragma omp parallel for schedule(dynamic, 1)
    for (long long chunk_low = 2;
         chunk_low < n;
         chunk_low += CHUNK_SIZE) {

        long long chunk_high =
            chunk_low + CHUNK_SIZE - 1;

        if (chunk_high >= n)
            chunk_high = n - 1;

        /*
         * Initially assume all numbers in this chunk are prime.
         */
        for (long long i = chunk_low;
             i <= chunk_high;
             i++) {

            is_prime[i] = 1;
        }

        /*
         * Use the base primes to remove composite numbers
         * from this chunk.
         */
        for (int i = 0; i < base_count; i++) {

            long long prime = base_primes[i];
            long long first_multiple = prime * prime;

            if (first_multiple < chunk_low) {
                first_multiple =
                    ((chunk_low + prime - 1) / prime) * prime;
            }

            for (long long multiple = first_multiple;
                 multiple <= chunk_high;
                 multiple += prime) {

                is_prime[multiple] = 0;
            }
        }
    }

    free(base_primes);

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