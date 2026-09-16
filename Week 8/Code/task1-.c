/**
 * @file Task1_MPI.c
 * @brief Open MPI implementation for finding prime numbers less than n
 *        using a segmented Sieve of Eratosthenes.
 *
 * The root process generates base primes up to sqrt(n), broadcasts them,
 * and divides the range [2, n) across MPI processes using balanced
 * quotient/remainder partitioning.
 *
 * Optional --task3 mode measures MPI communication overhead for
 * theoretical speedup analysis.
 *
 * @author Suchir
 * @author Zahra
 * @date 16/09/2026
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define DATA_TAG 0
#define READY_TAG 1
#define TIME_TAG 2

int main(int argc, char *argv[])
{
    int my_rank, p;
    int root = 0;
    int n = 0;
    int valid_input = 1;
    int task3_mode = 0;

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

    double communication_start, communication_end;
    double local_broadcast_time = 0.0;
    double max_broadcast_time = 0.0;
    double result_communication_time = 0.0;
    double total_communication_time = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /*
     * Root validates the input and enables Task 3 timing when requested.
     */
    if (my_rank == root) {
        if (argc != 2 && argc != 3) {
            printf("Error: insufficient arguments.\n");
            printf("Usage: %s <n> [--task3]\n", argv[0]);
            valid_input = 0;
        } else {
            n = atoi(argv[1]);

            if (n <= 0) {
                printf("Invalid input. n must be a positive integer.\n");
                valid_input = 0;
            }

            if (argc == 3) {
                if (strcmp(argv[2], "--task3") == 0) {
                    task3_mode = 1;
                } else {
                    printf("Invalid option: %s\n", argv[2]);
                    printf("Usage: %s <n> [--task3]\n", argv[0]);
                    valid_input = 0;
                }
            }
        }
    }

    MPI_Bcast(&valid_input, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (valid_input == 0) {
        MPI_Finalize();
        return 1;
    }

    MPI_Bcast(&task3_mode, 1, MPI_INT, root, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    start_overall = MPI_Wtime();

    if (task3_mode && p > 1)
        communication_start = MPI_Wtime();

    MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (task3_mode && p > 1) {
        communication_end = MPI_Wtime();
        local_broadcast_time += communication_end - communication_start;
    }

    if (n > 2)
        limit = (int)sqrt((double)(n - 1));
    else
        limit = 1;

    /*
     * Root generates the base primes up to sqrt(n).
     * These primes are later shared with every MPI process.
     */
    if (my_rank == root) {
        start_base = MPI_Wtime();

        base_sieve = malloc((size_t)(limit + 1) * sizeof(unsigned char));

        if (base_sieve == NULL) {
            printf("Root: base sieve memory allocation failed.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
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

        for (int i = 2; i <= limit; i++) {
            if (base_sieve[i] == 1)
                base_count++;
        }

        if (base_count > 0) {
            base_primes = malloc((size_t)base_count * sizeof(int));

            if (base_primes == NULL) {
                printf("Root: base prime memory allocation failed.\n");
                free(base_sieve);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
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
        base_time = end_base - start_base;
    }

    /*
     * Task 3 times the base-count broadcast separately.
     */
    if (task3_mode && p > 1) {
        MPI_Barrier(MPI_COMM_WORLD);
        communication_start = MPI_Wtime();
    }

    MPI_Bcast(
        &base_count,
        1,
        MPI_INT,
        root,
        MPI_COMM_WORLD
    );

    if (task3_mode && p > 1) {
        communication_end = MPI_Wtime();
        local_broadcast_time += communication_end - communication_start;
    }

    if (my_rank != root && base_count > 0) {
        base_primes = malloc((size_t)base_count * sizeof(int));

        if (base_primes == NULL) {
            printf("Rank %d: base prime allocation failed.\n", my_rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    /*
     * Base-prime data is broadcast after memory allocation so Task 3
     * communication timing excludes allocation overhead.
     */
    if (task3_mode && p > 1) {
        MPI_Barrier(MPI_COMM_WORLD);
        communication_start = MPI_Wtime();
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

    if (task3_mode && p > 1) {
        communication_end = MPI_Wtime();
        local_broadcast_time += communication_end - communication_start;
    }

    /*
     * Balanced quotient/remainder partitioning ensures each MPI process
     * receives either partition_size or partition_size + 1 values.
     */
    if (n > 2)
        total_numbers = (long long)n - 2;
    else
        total_numbers = 0;

    partition_size = total_numbers / p;
    partition_remainder = total_numbers % p;

    local_size = partition_size;

    if (my_rank < partition_remainder)
        local_size++;

    low = 2 + ((long long)my_rank * partition_size);

    if (my_rank < partition_remainder)
        low += my_rank;
    else
        low += partition_remainder;

    high = low + local_size - 1;

    if (local_size > 0) {
        local_is_prime =
            malloc((size_t)local_size * sizeof(unsigned char));

        if (local_is_prime == NULL) {
            printf("Rank %d: local sieve allocation failed.\n", my_rank);
            free(base_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    /*
     * Each process performs the segmented sieve only on its local range.
     */
    MPI_Barrier(MPI_COMM_WORLD);
    start_local = MPI_Wtime();

    for (long long i = 0; i < local_size; i++)
        local_is_prime[i] = 1;

    for (int i = 0; i < base_count; i++) {
        long long prime = base_primes[i];
        long long first_multiple = prime * prime;

        if (first_multiple < low) {
            first_multiple =
                ((low + prime - 1) / prime) * prime;
        }

        for (long long multiple = first_multiple;
             multiple <= high;
             multiple += prime) {
            local_is_prime[multiple - low] = 0;
        }
    }

    end_local = MPI_Wtime();
    local_time = end_local - start_local;

    /*
     * MPI_MAX is used because the parallel phase completes only when
     * the slowest MPI process finishes its local sieve.
     */
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
     * The root receives process segments in rank order.
     * Since each rank owns a contiguous increasing range, this preserves
     * the globally sorted prime-number output.
     */
    if (my_rank == root) {
        snprintf(
            file_name,
            sizeof(file_name),
            "primes_mpi_%d.txt",
            n
        );

        file = fopen(file_name, "w");

        if (file == NULL) {
            printf("Could not create output file.\n");
            free(local_is_prime);
            free(base_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        for (long long i = 0; i < local_size; i++) {
            if (local_is_prime[i] == 1) {
                long long number = low + i;

                fprintf(file, "%lld\n", number);

                if (n < 100)
                    printf("%lld ", number);
            }
        }

        for (int source = 1; source < p; source++) {
            long long source_size = partition_size;

            if (source < partition_remainder)
                source_size++;

            long long source_low =
                2 + ((long long)source * partition_size);

            if (source < partition_remainder)
                source_low += source;
            else
                source_low += partition_remainder;

            if (source_size > 0) {
                received_segment =
                    malloc((size_t)source_size * sizeof(unsigned char));

                if (received_segment == NULL) {
                    printf("Root: receive buffer allocation failed.\n");
                    fclose(file);
                    free(local_is_prime);
                    free(base_primes);

                    MPI_Abort(
                        MPI_COMM_WORLD,
                        EXIT_FAILURE
                    );
                }

                /*
                 * Task 3 measures the complete worker-to-root result
                 * transfer from the sender timestamp until the root has
                 * finished receiving the local segment.
                 */
                if (task3_mode) {
                    int ready = 1;
                    double sender_start;

                    MPI_Send(
                        &ready,
                        1,
                        MPI_INT,
                        source,
                        READY_TAG,
                        MPI_COMM_WORLD
                    );

                    MPI_Recv(
                        &sender_start,
                        1,
                        MPI_DOUBLE,
                        source,
                        TIME_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE
                    );

                    MPI_Recv(
                        received_segment,
                        (int)source_size,
                        MPI_UNSIGNED_CHAR,
                        source,
                        DATA_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE
                    );

                    double receiver_end = MPI_Wtime();

                    result_communication_time +=
                        receiver_end - sender_start;
                } else {
                    MPI_Recv(
                        received_segment,
                        (int)source_size,
                        MPI_UNSIGNED_CHAR,
                        source,
                        DATA_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE
                    );
                }

                for (long long i = 0; i < source_size; i++) {
                    if (received_segment[i] == 1) {
                        long long number = source_low + i;

                        fprintf(file, "%lld\n", number);

                        if (n < 100)
                            printf("%lld ", number);
                    }
                }

                free(received_segment);
                received_segment = NULL;
            }
        }

        fclose(file);

        if (n < 100)
            printf("\n");

        printf(
            "Prime numbers have been written to %s\n",
            file_name
        );
    } else {
        if (local_size > 0) {
            /*
             * In Task 3 mode, the worker waits until the root is ready,
             * records the sender timestamp, then sends the same segment
             * used during normal execution.
             */
            if (task3_mode) {
                int ready;
                double sender_start;

                MPI_Recv(
                    &ready,
                    1,
                    MPI_INT,
                    root,
                    READY_TAG,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );

                sender_start = MPI_Wtime();

                MPI_Send(
                    &sender_start,
                    1,
                    MPI_DOUBLE,
                    root,
                    TIME_TAG,
                    MPI_COMM_WORLD
                );

                MPI_Send(
                    local_is_prime,
                    (int)local_size,
                    MPI_UNSIGNED_CHAR,
                    root,
                    DATA_TAG,
                    MPI_COMM_WORLD
                );
            } else {
                MPI_Send(
                    local_is_prime,
                    (int)local_size,
                    MPI_UNSIGNED_CHAR,
                    root,
                    DATA_TAG,
                    MPI_COMM_WORLD
                );
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end_overall = MPI_Wtime();

    /*
     * Task 3 collects the maximum accumulated broadcast time.
     * This reduction happens after end_overall so the measurement
     * collection itself is excluded from overall execution time.
     */
    if (task3_mode) {
        MPI_Reduce(
            &local_broadcast_time,
            &max_broadcast_time,
            1,
            MPI_DOUBLE,
            MPI_MAX,
            root,
            MPI_COMM_WORLD
        );
    }

    if (my_rank == root) {
        overall_time = end_overall - start_overall;
        computational_time = base_time + max_local_time;

        printf("MPI processes: %d\n", p);

        printf(
            "Computational time only(s): %lf\n",
            computational_time
        );

        printf(
            "Overall time(s): %lf\n",
            overall_time
        );

        if (task3_mode) {
            if (p == 1) {
                max_broadcast_time = 0.0;
                result_communication_time = 0.0;
            }

            total_communication_time =
                max_broadcast_time +
                result_communication_time;

            printf(
                "Task 3 broadcast communication time(s): %lf\n",
                max_broadcast_time
            );

            printf(
                "Task 3 result communication time(s): %lf\n",
                result_communication_time
            );

            printf(
                "Task 3 total MPI communication time(s): %lf\n",
                total_communication_time
            );
        }
    }

    free(local_is_prime);
    free(base_primes);

    MPI_Finalize();
    return 0;
}