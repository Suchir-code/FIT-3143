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
 * Prime numbers are output in ascending order. Prime numbers that are less than
 * the input value of 100 will be output to the standard output whereas those larger
 * will be written into a text file.
 * 
 * GeeksforGeeks. (2024). Check for prime number. 
 * In GeeksforGeeks. https://www.geeksforgeeks.org/dsa/check-for-prime-number/
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
 * @brief Main entry point of the program
 * 
 * Takes in an input number and checks whether each odd number up to
 * the input is a prime.
 * Computation time is recorded for the duration of prime checking.
 * Overall time is recorded from after the user inputs N until the 
 * results are output or written to the file.
 * 
 * @return 0 if the program executes correctly
 * @return 1 if there is invalid input or memory allocation for files fail
 */
int main() {
    int n;
    struct timespec start, end, startComp, endComp;
    double time_taken;
    char filename[50];

    /**
     * Takes in user input N
     */
    printf("Enter the number: ");

    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input, please try again.\n");
        return 1;
    }

    // Start measuring overall execution time
    clock_gettime(CLOCK_MONOTONIC, &start);

    snprintf(filename, sizeof(filename), "primes_%d.txt", n);

    FILE *file = NULL;

    // Allocate memory to store prime results
    int *isPrime = malloc((size_t)n * sizeof(int));

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

    // Check every number p strictly less than n
    for (int p = 2; p < n; p++) {

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

        /**
         * Checks whether every odd number until p
         * is prime or not
         */
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

    // Stop measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &endComp);

    time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9;
    time_taken = (time_taken +
                 (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9;

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