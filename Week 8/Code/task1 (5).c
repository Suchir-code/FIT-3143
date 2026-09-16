/**

* @file Task2_Hybrid.c
* @brief Hybrid Open MPI + OpenMP implementation for finding prime numbers
* less than n using a segmented Sieve of Eratosthenes.
*
* The root process reads n and the number of OpenMP threads, then
* broadcasts these values to all MPI processes.
* The root finds the base primes up to sqrt(n), then broadcasts them.
* The range [2, n) is divided into contiguous segments so each MPI
* process performs the sieve only on its own section.
*
* Each MPI process further divides its local segment between its
* OpenMP threads.
*
* Performance timing is included for:
* * Base-prime generation
* * Base-prime broadcast
* * OpenMP sieve computation
* * Communication and output
* * Overall execution
*
* @author Suchir
* @author Zahra
* @date 12/09/2026
  */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define OUTPUT_BUFFER_SIZE 65536

int main(int argc, char *argv[])
{
int my_rank, p;
int root = 0;
int n = 0;
int valid_input = 1;
int threads = 0;

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

/*
 * Reusable output buffer.
 *
 * Instead of calling fprintf() once for every prime,
 * prime numbers are first stored in this buffer and
 * then written using fwrite() in larger chunks.
 */
char *output_buffer = NULL;
size_t output_buffer_used = 0;

/* Overall timing */
double start_overall;
double end_overall;
double overall_time;

/* Base-prime generation timing */
double start_base;
double end_base;
double base_time = 0.0;

/* Base-prime broadcast timing */
double start_broadcast;
double end_broadcast;
double broadcast_time = 0.0;

/* Local OpenMP computation timing */
double start_local;
double end_local;
double local_time;
double max_local_time;

/* Communication and output timing */
double start_output;
double end_output;
double output_time = 0.0;

double computational_time;

/* Initialise MPI */
MPI_Init(&argc, &argv);

MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &p);

/*
 * Root validates the command-line arguments.
 */
if (my_rank == root) {

    if (argc != 3) {
        printf("Usage: %s <n> <threads>\n", argv[0]);
        valid_input = 0;
    }
    else {
        n = atoi(argv[1]);
        threads = atoi(argv[2]);

        if (n <= 0 || threads <= 0) {
            printf("Invalid input.\n");
            valid_input = 0;
        }
    }
}

/*
 * Tell all MPI processes whether the input was valid.
 */
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

/*
 * Start measuring the overall execution time.
 */
MPI_Barrier(MPI_COMM_WORLD);
start_overall = MPI_Wtime();

/*
 * Broadcast n and the requested OpenMP thread count.
 */
MPI_Bcast(
    &n,
    1,
    MPI_INT,
    root,
    MPI_COMM_WORLD
);

MPI_Bcast(
    &threads,
    1,
    MPI_INT,
    root,
    MPI_COMM_WORLD
);

omp_set_num_threads(threads);

/*
 * Calculate the limit needed to find base primes.
 */
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

    /*
     * Initially assume all values are prime.
     */
    for (int i = 0; i <= limit; i++)
        base_sieve[i] = 1;

    if (limit >= 0)
        base_sieve[0] = 0;

    if (limit >= 1)
        base_sieve[1] = 0;

    /*
     * Standard Sieve of Eratosthenes for the
     * relatively small base-prime range.
     */
    for (
        int prime = 2;
        prime * prime <= limit;
        prime++
    ) {

        if (base_sieve[prime] == 1) {

            for (
                int multiple = prime * prime;
                multiple <= limit;
                multiple += prime
            ) {
                base_sieve[multiple] = 0;
            }
        }
    }

    /*
     * Count the base primes.
     */
    for (int i = 2; i <= limit; i++) {

        if (base_sieve[i] == 1)
            base_count++;
    }

    /*
     * Store the base primes in an array.
     */
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
    base_time = end_base - start_base;
}

/*
 * Broadcast the number of base primes and measure
 * the time taken to distribute the base-prime data.
 */
start_broadcast = MPI_Wtime();

MPI_Bcast(
    &base_count,
    1,
    MPI_INT,
    root,
    MPI_COMM_WORLD
);

if (my_rank != root && base_count > 0) {

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

end_broadcast = MPI_Wtime();
broadcast_time = end_broadcast - start_broadcast;

/*
 * Divide [2, n) between the MPI processes.
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

/*
 * Calculate the lower bound of this MPI process's segment.
 */
low =
    2 +
    ((long long)my_rank * partition_size);

if (my_rank < partition_remainder)
    low += my_rank;
else
    low += partition_remainder;

/*
 * Calculate the upper bound.
 */
high = low + local_size - 1;

/*
 * Allocate the local sieve.
 */
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

/*
 * Synchronise before starting the main sieve computation.
 */
MPI_Barrier(MPI_COMM_WORLD);

start_local = MPI_Wtime();

/*
 * Each MPI process divides its local segment
 * between its OpenMP threads.
 */
#pragma omp parallel
{
    int thread_id = omp_get_thread_num();
    int thread_count = omp_get_num_threads();

    /*
     * Divide the local MPI segment between threads.
     */
    long long thread_size =
        local_size / thread_count;

    long long thread_remainder =
        local_size % thread_count;

    long long thread_local_size =
        thread_size;

    if (thread_id < thread_remainder)
        thread_local_size++;

    /*
     * Calculate this thread's starting number.
     */
    long long thread_low =
        low +
        ((long long)thread_id * thread_size);

    if (thread_id < thread_remainder)
        thread_low += thread_id;
    else
        thread_low += thread_remainder;

    /*
     * Only perform work if this thread has numbers.
     */
    if (thread_local_size > 0) {

        long long thread_high =
            thread_low +
            thread_local_size -
            1;

        /*
         * Initially assume all numbers in this thread's
         * section are prime.
         */
        for (
            long long i = 0;
            i < thread_local_size;
            i++
        ) {
            local_is_prime[
                (thread_low - low) + i
            ] = 1;
        }

        /*
         * Mark composite numbers using the base primes.
         *
         * Since base_primes are sorted, once p^2 is greater
         * than the upper bound of this thread's section,
         * no further base primes can mark a number in this
         * section.
         */
        for (int i = 0; i < base_count; i++) {

            long long prime =
                base_primes[i];

            /*
             * Optimisation:
             * No multiple of this prime needs to be marked
             * if prime^2 is already beyond this section.
             */
            if (prime * prime > thread_high)
                break;

            /*
             * Start at prime^2, unless that is below
             * this thread's section.
             */
            long long first_multiple =
                prime * prime;

            if (first_multiple < thread_low) {

                first_multiple =
                    (
                        (thread_low + prime - 1)
                        / prime
                    ) * prime;
            }

            /*
             * Mark all multiples as composite.
             */
            for (
                long long multiple = first_multiple;
                multiple <= thread_high;
                multiple += prime
            ) {

                local_is_prime[
                    multiple - low
                ] = 0;
            }
        }
    }
}

end_local = MPI_Wtime();
local_time = end_local - start_local;

/*
 * Find the slowest MPI process's computation time.
 * This represents the parallel computation time.
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
 * Allocate the reusable receive buffer once.
 *
 * The largest MPI segment is partition_size + 1.
 * This avoids repeatedly allocating and freeing memory
 * for every source rank.
 */
if (my_rank == root && partition_size > 0) {

    received_segment =
        malloc(
            (size_t)(partition_size + 1) *
            sizeof(unsigned char)
        );

    if (received_segment == NULL) {

        printf(
            "Root: receive buffer allocation failed.\n"
        );

        free(local_is_prime);
        free(base_primes);

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );
    }
}

/*
 * Root creates one reusable output buffer.
 */
if (my_rank == root) {

    output_buffer =
        malloc(
            OUTPUT_BUFFER_SIZE
        );

    if (output_buffer == NULL) {

        printf(
            "Root: output buffer allocation failed.\n"
        );

        free(received_segment);
        free(local_is_prime);
        free(base_primes);

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );
    }
}

/*
 * Wait before beginning communication and output.
 */
MPI_Barrier(MPI_COMM_WORLD);

if (my_rank == root) {

    start_output = MPI_Wtime();

    snprintf(
        file_name,
        sizeof(file_name),
        "primes_hybrid_%d.txt",
        n
    );

    file = fopen(file_name, "w");

    if (file == NULL) {

        printf(
            "Root: unable to open output file.\n"
        );

        free(output_buffer);
        free(received_segment);
        free(local_is_prime);
        free(base_primes);

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );
    }

    /*
     * Write buffered output.
     *
     * This function-like block is repeated for each
     * segment. Prime numbers are appended to the buffer
     * and the buffer is written only when it becomes full.
     */

    /*
     * Write root's own segment first.
     */
    if (local_size > 0) {

        for (
            long long i = 0;
            i < local_size;
            i++
        ) {

            if (local_is_prime[i] == 1) {

                long long number =
                    low + i;

                int written =
                    snprintf(
                        output_buffer +
                        output_buffer_used,
                        OUTPUT_BUFFER_SIZE -
                        output_buffer_used,
                        "%lld\n",
                        number
                    );

                if (
                    written < 0 ||
                    (size_t)written >=
                    OUTPUT_BUFFER_SIZE -
                    output_buffer_used
                ) {

                    fwrite(
                        output_buffer,
                        1,
                        output_buffer_used,
                        file
                    );

                    output_buffer_used = 0;

                    written =
                        snprintf(
                            output_buffer,
                            OUTPUT_BUFFER_SIZE,
                            "%lld\n",
                            number
                        );
                }

                output_buffer_used +=
                    (size_t)written;

                if (
                    output_buffer_used >=
                    OUTPUT_BUFFER_SIZE - 32
                ) {

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

    /*
     * Receive and write the remaining MPI segments
     * in rank order.
     */
    for (int source = 1; source < p; source++) {

        long long source_size =
            partition_size;

        if (source < partition_remainder)
            source_size++;

        if (source_size > 0) {

            MPI_Recv(
                received_segment,
                (int)source_size,
                MPI_UNSIGNED_CHAR,
                source,
                0,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );

            /*
             * Calculate the starting number for this
             * source rank.
             */
            long long source_low =
                2 +
                ((long long)source * partition_size);

            if (source < partition_remainder)
                source_low += source;
            else
                source_low += partition_remainder;

            /*
             * Add this rank's primes to the output buffer.
             */
            for (
                long long i = 0;
                i < source_size;
                i++
            ) {

                if (received_segment[i] == 1) {

                    long long number =
                        source_low + i;

                    int written =
                        snprintf(
                            output_buffer +
                            output_buffer_used,
                            OUTPUT_BUFFER_SIZE -
                            output_buffer_used,
                            "%lld\n",
                            number
                        );

                    if (
                        written < 0 ||
                        (size_t)written >=
                        OUTPUT_BUFFER_SIZE -
                        output_buffer_used
                    ) {

                        fwrite(
                            output_buffer,
                            1,
                            output_buffer_used,
                            file
                        );

                        output_buffer_used = 0;

                        written =
                            snprintf(
                                output_buffer,
                                OUTPUT_BUFFER_SIZE,
                                "%lld\n",
                                number
                            );
                    }

                    output_buffer_used +=
                        (size_t)written;

                    if (
                        output_buffer_used >=
                        OUTPUT_BUFFER_SIZE - 32
                    ) {

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
    }

    /*
     * Flush any remaining output in the buffer.
     */
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

    output_time =
        end_output - start_output;
}
else {

    /*
     * Each non-root process sends its complete local
     * sieve segment to the root.
     */
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

/*
 * Wait for all processes to finish communication/output.
 */
MPI_Barrier(MPI_COMM_WORLD);

end_overall = MPI_Wtime();

overall_time =
    end_overall - start_overall;

/*
 * The computation includes:
 * - base-prime generation
 * - parallel OpenMP sieve
 *
 * Communication/output is kept separate for analysis,
 * while overall_time includes everything measured after
 * the initial barrier.
 */
if (my_rank == root) {

    computational_time =
        base_time +
        max_local_time;

    printf("\n");
    printf("==============================\n");
    printf("Hybrid MPI + OpenMP Results\n");
    printf("==============================\n");

    printf(
        "N: %d\n",
        n
    );

    printf(
        "MPI processes: %d\n",
        p
    );

    printf(
        "OpenMP threads per process: %d\n",
        threads
    );

    printf(
        "Total threads: %d\n",
        p * threads
    );

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
        "Communication + output: %.6f seconds\n",
        output_time
    );

    printf(
        "Overall execution: %.6f seconds\n",
        overall_time
    );

    printf(
        "Output file: %s\n",
        file_name
    );

    printf("==============================\n");
}

/*
 * Clean up allocated memory.
 */
free(output_buffer);
free(received_segment);
free(local_is_prime);
free(base_primes);

MPI_Finalize();

return 0;

}
