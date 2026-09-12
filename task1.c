/**
 * @file Task1.c
 * @brief Serial implementation for finding prime numbers less than n.
 *
 * This program accepts a positive integer n from the user and determines
 * all prime numbers that are strictly less than n using a serial algorithm.
 *
 * The program optimises prime checking by:
 * - Immediately identifying 2 as prime.
 * - Skipping all even numbers greater than 2.
 * - Checking only odd divisors.
 * - Limiting divisor checks to sqrt(p), where p is the number being tested.
 *
 * Prime numbers are output in ascending order. Prime numbers less than
 * 100 are printed to standard output, while larger results are written
 * to a text file.
 *
 * GeeksforGeeks. (2024). Check for prime number.
 * In GeeksforGeeks.
 * https://www.geeksforgeeks.org/dsa/check-for-prime-number/
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

/**
 * @brief Main entry point of the serial prime-number program.
 *
 * Reads an input number and checks every candidate strictly less than n
 * to determine whether it is prime.
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
    clock_gettime(CLOCK_MONOTONIC, &start);

    snprintf(file_name, sizeof(file_name), "primes_%d.txt", n);

    FILE *file = NULL;

    /* Allocate memory to store prime results. */
    int *is_prime = malloc((size_t)n * sizeof(int));

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
    clock_gettime(CLOCK_MONOTONIC, &start_comp);

    /* Check every number p strictly less than n. */
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
         * A composite number must have at least one divisor less than
         * or equal to its square root, so only divisors up to sqrt(p)
         * need to be checked.
         */
        int limit = (int)sqrt((double)p);

        /* Check only odd divisors. */
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
    clock_gettime(CLOCK_MONOTONIC, &end_comp);

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
     * Output is performed after computational timing.
     * Scanning from 2 to n guarantees ascending order.
     */
    for (int p = 2; p < n; p++) {

        if (is_prime[p] == 1) {

            if (n < 100) {
                printf("%d ", p);
            }
            else {
                fprintf(file, "%d\n", p);
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
    clock_gettime(CLOCK_MONOTONIC, &end);

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