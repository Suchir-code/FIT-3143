/**
 * @file Task2_Hybrid.c
 * @brief Hybrid Open MPI + OpenMP segmented Sieve of Eratosthenes.
 *
 * The global range [2, n) is divided across MPI processes using
 * quotient/remainder partitioning. Each MPI process then divides its
 * local range across OpenMP threads using the same balanced approach.
 *
 * Base primes up to sqrt(n) are generated once by the root and
 * broadcast to all MPI processes. The root collects result segments
 * in rank order and writes the final sorted output using buffered I/O.
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
#include <omp.h>

#define OUTPUT_BUFFER_SIZE 65536
#define DATA_TAG 0
#define READY_TAG 1
#define TIME_TAG 2

int main(int argc, char *argv[])
{
    int my_rank, p, root = 0, n = 0, valid_input = 1, threads = 0;
    int task3_mode = 0, limit, base_count = 0;
    int *base_primes = NULL;

    unsigned char *base_sieve = NULL;
    unsigned char *local_is_prime = NULL;
    unsigned char *received_segment = NULL;

    long long total_numbers, partition_size, partition_remainder;
    long long local_size, low, high;

    char file_name[50];
    FILE *file = NULL;

    char *output_buffer = NULL;
    size_t output_buffer_used = 0;

    double start_overall, end_overall, overall_time;
    double start_base, end_base, base_time = 0.0;
    double start_broadcast, end_broadcast, broadcast_time = 0.0;
    double start_local, end_local, local_time, max_local_time;
    double start_communication, end_communication;
    double communication_time = 0.0;
    double start_output, end_output, output_time = 0.0;
    double computational_time;

    double task3_start, task3_end;
    double task3_local_broadcast = 0.0;
    double task3_max_broadcast = 0.0;
    double task3_result_comm = 0.0;
    double task3_total_comm = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /*
     * Root validates n, thread count and optional Task 3 mode.
     */
    if (my_rank == root) {
        if (argc != 3 && argc != 4) {
            printf("Usage: %s <n> <threads> [--task3]\n", argv[0]);
            valid_input = 0;
        } else {
            n = atoi(argv[1]);
            threads = atoi(argv[2]);

            if (n <= 0 || threads <= 0) {
                printf("Invalid input.\n");
                valid_input = 0;
            }

            if (argc == 4) {
                if (strcmp(argv[3], "--task3") == 0) {
                    task3_mode = 1;
                } else {
                    printf("Invalid option: %s\n", argv[3]);
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
        task3_start = MPI_Wtime();

    MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (task3_mode && p > 1) {
        task3_end = MPI_Wtime();
        task3_local_broadcast += task3_end - task3_start;
    }

    if (task3_mode && p > 1)
        task3_start = MPI_Wtime();

    MPI_Bcast(&threads, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (task3_mode && p > 1) {
        task3_end = MPI_Wtime();
        task3_local_broadcast += task3_end - task3_start;
    }

    omp_set_num_threads(threads);

    if (n > 2)
        limit = (int)sqrt((double)(n - 1));
    else
        limit = 1;

    /*
     * Root generates the base primes required by every MPI process.
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
                if (base_sieve[i] == 1)
                    base_primes[index++] = i;
            }
        }

        free(base_sieve);

        end_base = MPI_Wtime();
        base_time = end_base - start_base;
    }

    start_broadcast = MPI_Wtime();

    /*
     * Task 3 measures base-prime communication separately from
     * computation.
     */
    if (task3_mode && p > 1) {
        MPI_Barrier(MPI_COMM_WORLD);
        task3_start = MPI_Wtime();
    }

    MPI_Bcast(&base_count, 1, MPI_INT, root, MPI_COMM_WORLD);

    if (task3_mode && p > 1) {
        task3_end = MPI_Wtime();
        task3_local_broadcast += task3_end - task3_start;
    }

    if (my_rank != root && base_count > 0) {
        base_primes = malloc((size_t)base_count * sizeof(int));

        if (base_primes == NULL) {
            printf("Rank %d: base prime allocation failed.\n", my_rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    /*
     * Memory allocation is completed before timing the base-prime
     * broadcast so allocation overhead is excluded from Task 3
     * communication measurements.
     */
    if (task3_mode && p > 1) {
        MPI_Barrier(MPI_COMM_WORLD);
        task3_start = MPI_Wtime();
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
        task3_end = MPI_Wtime();
        task3_local_broadcast += task3_end - task3_start;
    }

    end_broadcast = MPI_Wtime();
    broadcast_time = end_broadcast - start_broadcast;

    /*
     * First level of parallelism: divide [2, n) between MPI processes.
     * Quotient/remainder partitioning keeps workloads balanced.
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
        local_is_prime = malloc((size_t)local_size * sizeof(unsigned char));

        if (local_is_prime == NULL) {
            printf("Rank %d: local sieve allocation failed.\n", my_rank);
            free(base_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    /*
     * Second level of parallelism: each MPI process divides its local
     * range between OpenMP threads. Threads write only to their own
     * non-overlapping section of local_is_prime.
     */
    MPI_Barrier(MPI_COMM_WORLD);
    start_local = MPI_Wtime();

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int thread_count = omp_get_num_threads();

        long long thread_size = local_size / thread_count;
        long long thread_remainder = local_size % thread_count;
        long long thread_local_size = thread_size;

        if (thread_id < thread_remainder)
            thread_local_size++;

        long long thread_low =
            low + ((long long)thread_id * thread_size);

        if (thread_id < thread_remainder)
            thread_low += thread_id;
        else
            thread_low += thread_remainder;

        if (thread_local_size > 0) {
            long long thread_high =
                thread_low + thread_local_size - 1;

            for (long long i = 0; i < thread_local_size; i++) {
                local_is_prime[(thread_low - low) + i] = 1;
            }

            for (int i = 0; i < base_count; i++) {
                long long prime = base_primes[i];

                if (prime * prime > thread_high)
                    break;

                long long first_multiple = prime * prime;

                if (first_multiple < thread_low) {
                    first_multiple =
                        ((thread_low + prime - 1) / prime) * prime;
                }

                for (long long multiple = first_multiple;
                     multiple <= thread_high;
                     multiple += prime) {
                    local_is_prime[multiple - low] = 0;
                }
            }
        }
    }

    end_local = MPI_Wtime();
    local_time = end_local - start_local;

    /*
     * MPI_MAX captures the slowest MPI process because the hybrid
     * computation phase completes only when all processes finish.
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

    if (my_rank == root && partition_size > 0) {
        received_segment =
            malloc((size_t)(partition_size + 1) * sizeof(unsigned char));

        if (received_segment == NULL) {
            printf("Root: receive buffer allocation failed.\n");
            free(local_is_prime);
            free(base_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    /*
     * Buffered output reduces the overhead of performing an individual
     * file write for every prime number.
     */
    if (my_rank == root) {
        output_buffer = malloc(OUTPUT_BUFFER_SIZE);

        if (output_buffer == NULL) {
            printf("Root: output buffer allocation failed.\n");
            free(received_segment);
            free(local_is_prime);
            free(base_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (my_rank == root) {
        snprintf(
            file_name,
            sizeof(file_name),
            "primes_hybrid_%d.txt",
            n
        );

        file = fopen(file_name, "w");

        if (file == NULL) {
            printf("Root: unable to open output file.\n");
            free(output_buffer);
            free(received_segment);
            free(local_is_prime);
            free(base_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        start_output = MPI_Wtime();

        if (local_size > 0) {
            for (long long i = 0; i < local_size; i++) {
                if (local_is_prime[i] == 1) {
                    long long number = low + i;

                    int written = snprintf(
                        output_buffer + output_buffer_used,
                        OUTPUT_BUFFER_SIZE - output_buffer_used,
                        "%lld\n",
                        number
                    );

                    if (written < 0 ||
                        (size_t)written >=
                        OUTPUT_BUFFER_SIZE - output_buffer_used) {

                        fwrite(
                            output_buffer,
                            1,
                            output_buffer_used,
                            file
                        );

                        output_buffer_used = 0;

                        written = snprintf(
                            output_buffer,
                            OUTPUT_BUFFER_SIZE,
                            "%lld\n",
                            number
                        );
                    }

                    output_buffer_used += (size_t)written;

                    if (output_buffer_used >=
                        OUTPUT_BUFFER_SIZE - 32) {

                        fwrite(
                            output_buffer,
                            1,
                            output_buffer_used,
                            file
                        );

                        output_buffer_used = 0;
                    }
                }
            }
        }

        end_output = MPI_Wtime();
        output_time = end_output - start_output;

        /*
         * Segments are received in rank order. Since each rank owns a
         * contiguous increasing range, the final output remains sorted.
         */
        for (int source = 1; source < p; source++) {
            long long source_size = partition_size;

            if (source < partition_remainder)
                source_size++;

            if (source_size > 0) {
                /*
                 * Task 3 measures complete worker-to-root result
                 * communication from sender timestamp to receiver
                 * completion.
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

                    start_communication = MPI_Wtime();

                    MPI_Recv(
                        received_segment,
                        (int)source_size,
                        MPI_UNSIGNED_CHAR,
                        source,
                        DATA_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE
                    );

                    end_communication = MPI_Wtime();

                    communication_time +=
                        end_communication - start_communication;

                    double receiver_end = MPI_Wtime();

                    task3_result_comm +=
                        receiver_end - sender_start;
                } else {
                    start_communication = MPI_Wtime();

                    MPI_Recv(
                        received_segment,
                        (int)source_size,
                        MPI_UNSIGNED_CHAR,
                        source,
                        DATA_TAG,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE
                    );

                    end_communication = MPI_Wtime();

                    communication_time +=
                        end_communication - start_communication;
                }
            }

            start_output = MPI_Wtime();

            long long source_low =
                2 + ((long long)source * partition_size);

            if (source < partition_remainder)
                source_low += source;
            else
                source_low += partition_remainder;

            for (long long i = 0; i < source_size; i++) {
                if (received_segment[i] == 1) {
                    long long number = source_low + i;

                    int written = snprintf(
                        output_buffer + output_buffer_used,
                        OUTPUT_BUFFER_SIZE - output_buffer_used,
                        "%lld\n",
                        number
                    );

                    if (written < 0 ||
                        (size_t)written >=
                        OUTPUT_BUFFER_SIZE - output_buffer_used) {

                        fwrite(
                            output_buffer,
                            1,
                            output_buffer_used,
                            file
                        );

                        output_buffer_used = 0;

                        written = snprintf(
                            output_buffer,
                            OUTPUT_BUFFER_SIZE,
                            "%lld\n",
                            number
                        );
                    }

                    output_buffer_used += (size_t)written;

                    if (output_buffer_used >=
                        OUTPUT_BUFFER_SIZE - 32) {

                        fwrite(
                            output_buffer,
                            1,
                            output_buffer_used,
                            file
                        );

                        output_buffer_used = 0;
                    }
                }
            }

            end_output = MPI_Wtime();
            output_time += end_output - start_output;
        }

        /*
         * Flush any remaining values in the output buffer before
         * closing the file.
         */
        start_output = MPI_Wtime();

        if (output_buffer_used > 0) {
            fwrite(
                output_buffer,
                1,
                output_buffer_used,
                file
            );
        }

        fclose(file);

        end_output = MPI_Wtime();
        output_time += end_output - start_output;
    } else {
        if (local_size > 0) {
            /*
             * Task 3 uses a ready handshake before recording the sender
             * timestamp so waiting for the root is not counted as result
             * transfer time.
             */
            if (task3_mode) {
                int ready;

                MPI_Recv(
                    &ready,
                    1,
                    MPI_INT,
                    root,
                    READY_TAG,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );

                double sender_start = MPI_Wtime();

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

    /*
     * Overall wall-clock time includes computation, MPI communication
     * and file output.
     */
    MPI_Barrier(MPI_COMM_WORLD);
    end_overall = MPI_Wtime();

    overall_time =
        end_overall - start_overall;

    /*
     * Task 3 uses the maximum accumulated broadcast time across MPI
     * processes to represent the slowest communication path.
     */
    if (task3_mode) {
        MPI_Reduce(
            &task3_local_broadcast,
            &task3_max_broadcast,
            1,
            MPI_DOUBLE,
            MPI_MAX,
            root,
            MPI_COMM_WORLD
        );
    }

    if (my_rank == root) {
        computational_time =
            base_time + max_local_time;

        printf("\n");
        printf("==============================\n");
        printf("Hybrid MPI + OpenMP Results\n");
        printf("==============================\n");

        printf("N: %d\n", n);
        printf("MPI processes: %d\n", p);

        printf(
            "OpenMP threads per process: %d\n",
            threads
        );

        printf("Total threads: %d\n", p * threads);

        printf(
            "Base-prime generation: %.6f seconds\n",
            base_time
        );

        printf(
            "Base-prime broadcast: %.6f seconds\n",
            broadcast_time
        );

        printf(
            "OpenMP sieve computation: %.6f seconds\n",
            max_local_time
        );

        printf(
            "Computational total: %.6f seconds\n",
            computational_time
        );

        printf(
            "MPI communication: %.6f seconds\n",
            communication_time
        );

        printf(
            "File output: %.6f seconds\n",
            output_time
        );

        printf(
            "Overall execution: %.6f seconds\n",
            overall_time
        );

        printf("Output file: %s\n", file_name);

        /*
         * Broadcast and worker-to-root communication are combined for
         * the Task 3 theoretical speedup calculation.
         */
        if (task3_mode) {
            if (p == 1) {
                task3_max_broadcast = 0.0;
                task3_result_comm = 0.0;
            }

            task3_total_comm =
                task3_max_broadcast +
                task3_result_comm;

            printf(
                "Task 3 broadcast communication: %.6f seconds\n",
                task3_max_broadcast
            );

            printf(
                "Task 3 result communication: %.6f seconds\n",
                task3_result_comm
            );

            printf(
                "Task 3 total MPI communication: %.6f seconds\n",
                task3_total_comm
            );
        }

        printf("==============================\n");
    }

    free(output_buffer);
    free(received_segment);
    free(local_is_prime);
    free(base_primes);

    MPI_Finalize();

    return 0;
}