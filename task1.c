/**
 * @file Task1.c
 * @brief Serial implementation for finding prime numbers less than n.
 *
 * This program accepts a positive integer n from the user and determines
 * all prime numbers that are strictly less than n using the Sieve of
 * Eratosthenes.
 *
 * The program optimises prime searching by marking multiples of each
 * discovered prime as composite rather than testing each number separately.
 *
 * Prime numbers are output in ascending order. Prime numbers less than
 * 100 are printed to standard output, while larger results are written
 * to a text file.
 *
 * @author Suchir
 * @author Zahra
 * @date 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Main entry point of the serial prime-number program.
 *
 * Reads an input number and uses the Sieve of Eratosthenes
 * to determine all prime numbers strictly less than n.
 *
 * Computational time measures only the prime-number calculation.
 * Overall time includes result output and file writing.
 *
 * @return 0 if the program executes successfully.
 * @return 1 if invalid input, memory allocation, or file creation fails.
 */

int main()
{
    int n;

    struct timespec start, end, start_comp, end_comp;

    double time_taken;

    char file_name[50];

    /* Take user input. */
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
        "primes_%d.txt",
        n
    );


    FILE *file = NULL;


    /*
     * Allocate memory for the sieve.
     *
     * 1 means the number is currently considered prime.
     * 0 means the number is composite.
     */
    int *is_prime =
        malloc((size_t)n * sizeof(int));


    if (is_prime == NULL) {

        printf("Memory allocation failed.\n");

        return 1;
    }


    /* Create output file for n >= 100. */
    if (n >= 100) {

        file = fopen(
            file_name,
            "w"
        );


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


    /*
     * Initially assume all numbers are prime.
     */
    for (int i = 0; i < n; i++) {

        is_prime[i] = 1;
    }


    /*
     * 0 and 1 are not prime.
     */
    if (n > 0) {

        is_prime[0] = 0;
    }


    if (n > 1) {

        is_prime[1] = 0;
    }


    /**
     * Sieve of Eratosthenes.
     *
     * If p is prime, all multiples of p starting from p * p
     * are composite.
     *
     * Values below p * p have already been handled by smaller
     * prime factors.
     */
    for (
        int p = 2;
        p * p < n;
        p++
    ) {

        if (is_prime[p] == 1) {

            for (
                int multiple = p * p;
                multiple < n;
                multiple += p
            ) {

                is_prime[multiple] = 0;
            }
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


    /**
     * Output is performed after computational timing.
     * Scanning from 2 to n guarantees ascending order.
     */
    for (
        int p = 2;
        p < n;
        p++
    ) {

        if (is_prime[p] == 1) {

            if (n < 100) {

                printf(
                    "%d ",
                    p
                );
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