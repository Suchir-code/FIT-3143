#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_THREADS 16
#define CHUNK_SIZE 10000

int main() {
    int n;
    struct timespec start, end, startComp, endComp;
    double time_taken;
    char filename[50];

    printf("Enter the number: ");

    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input, please try again.\n");
        return 1;
    }

    // Start measuring overall execution time
    clock_gettime(CLOCK_MONOTONIC, &start);

    snprintf(filename, sizeof(filename), "primes3_%d.txt", n);

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

    // Make OpenMP use the same number of threads as Task 2
    omp_set_dynamic(0);
    omp_set_num_threads(NUM_THREADS);

    // Start measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &startComp);

    // Parallel prime number computation
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE)
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