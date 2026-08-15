#include <math.h>  
#include <stdbool.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <time.h>  

//added for task 3
#include <omp.h>
  
int main() {  
    int n;  
    struct timespec start, end, startComp, endComp;  
    double time_taken;  
  
 
    printf("Enter the number: ");  
 
    // Start measuring overall execution time 
  
    if (scanf("%d", &n) != 1 || n <= 0) {  
        printf("Invalid input, please try again.\n");  
        return 1;  
    }  
 
    clock_gettime(CLOCK_MONOTONIC, &start); 

    //added for task 3
    int *isPrime = malloc(n * sizeof(int)); 

    //added for task 3
    if (isPrime == NULL) { 
        printf("Memory allocation failed.\n"); 
        return 1; 
    } 
 
    FILE *file = NULL;  
  
    // For n greater than 100 we create a file  
    if (n >= 100) {  
        file = fopen("text.txt", "w");  
  
        if (file == NULL) {  
            printf("Could not create file.\n");

            //added for task 3
            free(isPrime);

            return 1;  
        }  
    }  
  
    // Start measuring computational time 
    clock_gettime(CLOCK_MONOTONIC, &startComp);  
  
    // Check every number p strictly less than n  

    //added for task 3
    #pragma omp parallel for 
    for (int p = 2; p < n; p++) {  
        int cnt = 0;  

        //added for task 3
        isPrime[p] = 0;
  
        // 2 is prime  
        if (p == 2) {

            //added for task 3
            isPrime[p] = 1;
  
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

            //added for task 3
            isPrime[p] = 1;
        }  
    }  
  
    // Stop measuring computational time 
    clock_gettime(CLOCK_MONOTONIC, &endComp);  
  
    time_taken = (endComp.tv_sec - startComp.tv_sec) * 1e9;  
    time_taken = (time_taken +  
                 (endComp.tv_nsec - startComp.tv_nsec)) * 1e-9;  
  
    printf("\nComputational time only(s): %lf\n", time_taken);  

    //added for task 3
    for (int p = 2; p < n; p++) {

        if (isPrime[p] == 1) {

            if (n < 100) {
                printf("%d\n", p);
            } else {
                fprintf(file, "%d\n", p);
            }
        }
    }
  
    if (n < 100) {  
        printf("\n");  
    } else {  
        fclose(file);  
        printf("Prime numbers have been written to text.txt\n");  
    }  

    //added for task 3
    free(isPrime);
  
    // Stop measuring overall execution time 
    clock_gettime(CLOCK_MONOTONIC, &end);  
  
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;  
    time_taken = (time_taken +  
                 (end.tv_nsec - start.tv_nsec)) * 1e-9;  
  
    printf("Overall time(s): %lf\n", time_taken);  
  
    return 0;  
}