#include <math.h> 
#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
 
int main() { 
    int n; 
    int cnt = 0; 
    struct timespec start, end, startComp, endComp; 
    double time_taken; 
 

    printf("Enter the number: "); 

    // Start measuring overall execution time
 
    if (scanf("%d", &n) != 1 || n <= 0) { 
        printf("Invalid input, please try again.\n"); 
        return 1; 
    } 

    clock_gettime(CLOCK_MONOTONIC, &start); 

    FILE *file = NULL; 
 
    // For n greater than 100 we create a file 
    if (n >= 100) { 
        file = fopen("text.txt", "w"); 
 
        if (file == NULL) { 
            printf("Could not create file.\n"); 
            return 1; 
        } 
    } 
 
    // Start measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &startComp); 
 
    // Check every number p strictly less than n 
    for (int p = 2; p < n; p++) { 
 
        cnt = 0; 
 
        // 2 is prime 
        if (p == 2) { 
            if (n < 100) { 
                printf("%d ", p); 
            } else { 
                fprintf(file, "%d\n", p); 
            } 
 
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
 
        // If cnt is 0, p is prime 
        if (cnt == 0) { 
 
            if (n < 100) { 
                printf("%d", p); 
            } else { 
                fprintf(file, "%d\n", p); 
            } 
        } 
    } 
 
    // Stop measuring computational time
    clock_gettime(CLOCK_MONOTONIC, &endComp); 
 
    time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9; 
    time_taken = (time_taken + 
                 (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9; 
 
    printf("\nComputational time only(s): %lf\n", time_taken); 
 
    if (n < 100) { 
        printf("\n"); 
    } else { 
        fclose(file); 
        printf("Prime numbers have been written to text.txt\n"); 
    } 
 
    // Stop measuring overall execution time
    clock_gettime(CLOCK_MONOTONIC, &end); 
 
    time_taken = (end.tv_sec - start.tv_sec) * 1e9; 
    time_taken = (time_taken + 
                 (end.tv_nsec - start.tv_nsec)) * 1e-9; 
 
    printf("Overall time(s): %lf\n", time_taken); 
 
    return 0; 
}
