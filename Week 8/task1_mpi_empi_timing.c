/**
 * @file task1_mpi_empi_timing.c
 * @brief Open MPI implementation for finding prime numbers less than n
 * using a segmented Sieve of Eratosthenes.
 *
 * The root process reads n and broadcasts it to all MPI processes.
 * The root finds the base primes up to sqrt(n), then broadcasts them.
 * The range [2, n) is divided into contiguous segments so each MPI
 * process performs the sieve only on its own section.
 *
 * This reduces memory usage and communication compared with storing
 * and reducing a complete n-element sieve on every MPI process.
 *
 * Additional timing measurements are included to determine the
 * serial and parallel fractions required for Amdahl's Law.
 *
 * @author Suchir
 * @author Zahra
 * @date 12/09/2026
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int my_rank, p;
    int root = 0;
    int n = 0;
    int valid_input = 1;
    int limit;
    int base_count = 0;

    int *base_primes = NULL;

    unsigned char *base_sieve = NULL;
    unsigned char *local_is_prime = NULL;
    unsigned char *received_segment = NULL;

    long long total_numbers;
    long long partition_size;
    long long partition_remainder;
    long long local_size;
    long long low;
    long long high;

    char file_name[50];
    FILE *file = NULL;

    double start_overall, end_overall, overall_time;

    double start_base, end_base, base_time = 0.0;

    double start_local, end_local;
    double local_time, max_local_time;

    double computational_time;

    /*
     * Additional measurements for Amdahl's Law.
     */
    double serial_time;
    double parallel_time;
    double serial_fraction;
    double parallel_fraction;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == root) {
        if (argc != 2) {
            printf("Error: insufficient arguments.\n");
            printf("Usage: %s <n>\n", argv[0]);

            valid_input = 0;
        }
        else {
            n = atoi(argv[1]);

            if (n <= 0) {
                printf(
                    "Invalid input. n must be a positive integer.\n"
                );

                valid_input = 0;
            }
        }
    }

    MPI_Bcast(
        &valid_input,
        1,
        MPI_INT,
        root,
        MPI_COMM_WORLD
    );

    if (valid_input == 0) {
        MPI_Finalize();
        return 1;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    start_overall = MPI_Wtime();

    MPI_Bcast(
        &n,
        1,
        MPI_INT,
        root,
        MPI_COMM_WORLD
    );

    if (n > 2)
        limit = (int)sqrt((double)(n - 1));
    else
        limit = 1;

    /*
     * Root generates the base primes up to sqrt(n).
     */
    if (my_rank == root) {

        start_base = MPI_Wtime();

        base_sieve =
            malloc(
                (size_t)(limit + 1) *
                sizeof(unsigned char)
            );

        if (base_sieve == NULL) {
            printf(
                "Root: base sieve memory allocation failed.\n"
            );

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );
        }

        for (int i = 0; i <= limit; i++)
            base_sieve[i] = 1;

        if (limit >= 0)
            base_sieve[0] = 0;

        if (limit >= 1)
            base_sieve[1] = 0;

        for (int prime = 2;
             prime * prime <= limit;
             prime++) {

            if (base_sieve[prime] == 1) {

                for (int multiple = prime * prime;
                     multiple <= limit;
                     multiple += prime) {

                    base_sieve[multiple] = 0;
                }
            }
        }

        for (int i = 2; i <= limit; i++) {

            if (base_sieve[i] == 1)
                base_count++;
        }

        if (base_count > 0) {

            base_primes =
                malloc(
                    (size_t)base_count *
                    sizeof(int)
                );

            if (base_primes == NULL) {

                printf(
                    "Root: base prime memory allocation failed.\n"
                );

                free(base_sieve);

                MPI_Abort(
                    MPI_COMM_WORLD,
                    EXIT_FAILURE
                );
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

        end_base = MPI_Wtime();

        base_time =
            end_base - start_base;
    }

    MPI_Bcast(
        &base_count,
        1,
        MPI_INT,
        root,
        MPI_COMM_WORLD
    );

    if (my_rank != root &&
        base_count > 0) {

        base_primes =
            malloc(
                (size_t)base_count *
                sizeof(int)
            );

        if (base_primes == NULL) {

            printf(
                "Rank %d: base prime allocation failed.\n",
                my_rank
            );

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );
        }
    }

    if (base_count > 0) {

        MPI_Bcast(
            base_primes,
            base_count,
            MPI_INT,
            root,
            MPI_COMM_WORLD
        );
    }

    /*
     * Divide [2, n) between the MPI processes.
     */
    if (n > 2)
        total_numbers =
            (long long)n - 2;
    else
        total_numbers = 0;

    partition_size =
        total_numbers / p;

    partition_remainder =
        total_numbers % p;

    local_size =
        partition_size;

    if (my_rank < partition_remainder)
        local_size++;

    low =
        2 +
        ((long long)my_rank *
        partition_size);

    if (my_rank < partition_remainder)
        low += my_rank;
    else
        low += partition_remainder;

    high =
        low + local_size - 1;

    if (local_size > 0) {

        local_is_prime =
            malloc(
                (size_t)local_size *
                sizeof(unsigned char)
            );

        if (local_is_prime == NULL) {

            printf(
                "Rank %d: local sieve allocation failed.\n",
                my_rank
            );

            free(base_primes);

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    start_local =
        MPI_Wtime();

    for (long long i = 0;
         i < local_size;
         i++) {

        local_is_prime[i] = 1;
    }

    /*
     * Each process sieves only its own segment.
     */
    for (int i = 0;
         i < base_count;
         i++) {

        long long prime =
            base_primes[i];

        long long first_multiple =
            prime * prime;

        if (first_multiple < low) {

            first_multiple =
                ((low + prime - 1) /
                prime) * prime;
        }

        for (long long multiple =
                 first_multiple;
             multiple <= high;
             multiple += prime) {

            local_is_prime[
                multiple - low
            ] = 0;
        }
    }

    end_local =
        MPI_Wtime();

    local_time =
        end_local - start_local;

    MPI_Reduce(
        &local_time,
        &max_local_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        root,
        MPI_COMM_WORLD
    );

    /*
     * Root writes the segments in rank order.
     */
    if (my_rank == root) {

        snprintf(
            file_name,
            sizeof(file_name),
            "primes_mpi_%d.txt",
            n
        );

        file =
            fopen(
                file_name,
                "w"
            );

        if (file == NULL) {

            printf(
                "Could not create output file.\n"
            );

            free(local_is_prime);
            free(base_primes);

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );
        }

        for (long long i = 0;
             i < local_size;
             i++) {

            if (local_is_prime[i] == 1) {

                long long number =
                    low + i;

                fprintf(
                    file,
                    "%lld\n",
                    number
                );

                if (n < 100)
                    printf(
                        "%lld ",
                        number
                    );
            }
        }

        for (int source = 1;
             source < p;
             source++) {

            long long source_size =
                partition_size;

            if (source <
                partition_remainder) {

                source_size++;
            }

            long long source_low =
                2 +
                ((long long)source *
                partition_size);

            if (source <
                partition_remainder) {

                source_low +=
                    source;
            }
            else {
                source_low +=
                    partition_remainder;
            }

            if (source_size > 0) {

                received_segment =
                    malloc(
                        (size_t)source_size *
                        sizeof(unsigned char)
                    );

                if (received_segment ==
                    NULL) {

                    printf(
                        "Root: receive buffer allocation failed.\n"
                    );

                    fclose(file);

                    free(
                        local_is_prime
                    );

                    free(
                        base_primes
                    );

                    MPI_Abort(
                        MPI_COMM_WORLD,
                        EXIT_FAILURE
                    );
                }

                MPI_Recv(
                    received_segment,
                    (int)source_size,
                    MPI_UNSIGNED_CHAR,
                    source,
                    0,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );

                for (long long i = 0;
                     i < source_size;
                     i++) {

                    if (
                        received_segment[i]
                        == 1
                    ) {

                        long long number =
                            source_low + i;

                        fprintf(
                            file,
                            "%lld\n",
                            number
                        );

                        if (n < 100)
                            printf(
                                "%lld ",
                                number
                            );
                    }
                }

                free(
                    received_segment
                );

                received_segment =
                    NULL;
            }
        }

        fclose(file);

        if (n < 100)
            printf("\n");

        printf(
            "Prime numbers have been written to %s\n",
            file_name
        );
    }
    else {

        if (local_size > 0) {

            MPI_Send(
                local_is_prime,
                (int)local_size,
                MPI_UNSIGNED_CHAR,
                root,
                0,
                MPI_COMM_WORLD
            );
        }
    }

    MPI_Barrier(
        MPI_COMM_WORLD
    );

    end_overall =
        MPI_Wtime();

    if (my_rank == root) {

        overall_time =
            end_overall -
            start_overall;

        computational_time =
            base_time +
            max_local_time;

        /*
         * Amdahl's Law timing.
         *
         * The local segmented sieve is the parallelisable
         * portion of the program.
         *
         * Everything remaining in the overall execution
         * time is treated as the serial/non-parallel portion.
         *
         * Run with one MPI process when deriving S and P.
         */
        parallel_time =
            max_local_time;

        serial_time =
            overall_time -
            parallel_time;

        serial_fraction =
            serial_time /
            overall_time;

        parallel_fraction =
            parallel_time /
            overall_time;

        printf(
            "MPI processes: %d\n",
            p
        );

        printf(
            "Computational time only(s): %lf\n",
            computational_time
        );

        printf(
            "Overall time(s): %lf\n",
            overall_time
        );

        printf(
            "Parallel portion time(s): %lf\n",
            parallel_time
        );

        printf(
            "Serial/non-parallel portion time(s): %lf\n",
            serial_time
        );

        printf(
            "Serial fraction (S): %lf\n",
            serial_fraction
        );

        printf(
            "Parallel fraction (P): %lf\n",
            parallel_fraction
        );
    }

    free(
        local_is_prime
    );

    free(
        base_primes
    );

    MPI_Finalize();

    return 0;
}