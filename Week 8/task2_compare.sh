#!/bin/bash

# ============================================================
# TASK 2 - MPI + OPENMP AUTOMATED PERFORMANCE TEST
# ============================================================

# ------------------------------------------------------------
# SETTINGS
# ------------------------------------------------------------

SERIAL_EXEC="../task1"
MPI_EXEC="./task1_mpi"
HYBRID_EXEC="./task2_hybrid"

RESULTS="task2_results.csv"

# 30 values of n
tests=(
    10000000
    20000000
    30000000
    40000000
    50000000
    60000000
    70000000
    80000000
    90000000
    100000000
    110000000
    120000000
    130000000
    140000000
    150000000
    160000000
    170000000
    180000000
    190000000
    200000000
    210000000
    220000000
    230000000
    240000000
    250000000
    260000000
    270000000
    280000000
    290000000
    300000000
)

# ------------------------------------------------------------
# HYBRID CONFIGURATIONS
# ------------------------------------------------------------

# Format:
# MPI_PROCESSES:OPENMP_THREADS

configs=(
    "1:1"
    "1:2"
    "2:1"
    "1:4"
    "2:2"
    "4:1"
    "1:8"
    "2:4"
    "4:2"
    "8:1"
)

# ------------------------------------------------------------
# CHECK EXECUTABLES
# ------------------------------------------------------------

echo "Checking executables..."

if [ ! -x "$SERIAL_EXEC" ]; then
    echo "ERROR: Serial executable not found: $SERIAL_EXEC"
    exit 1
fi

if [ ! -x "$MPI_EXEC" ]; then
    echo "ERROR: MPI executable not found: $MPI_EXEC"
    exit 1
fi

if [ ! -x "$HYBRID_EXEC" ]; then
    echo "ERROR: Hybrid executable not found: $HYBRID_EXEC"
    exit 1
fi

echo "All executables found."
echo

# ------------------------------------------------------------
# CSV HEADER
# ------------------------------------------------------------

echo "N,MPI_Processes,OpenMP_Threads,Total_Threads,Serial_Overall,MPI_Overall,Hybrid_Computational,Hybrid_Overall,Serial_Speedup,MPI_Speedup" > "$RESULTS"

# ------------------------------------------------------------
# MAIN TEST LOOP
# ------------------------------------------------------------

for n in "${tests[@]}"; do

    echo "============================================================"
    echo "N = $n"
    echo "============================================================"

    # --------------------------------------------------------
    # SERIAL
    # --------------------------------------------------------

    echo "Running Serial..."

    serial_output=$("$SERIAL_EXEC" "$n")

    serial_overall=$(echo "$serial_output" |
        grep "Overall time" |
        awk '{print $NF}')

    if [ -z "$serial_overall" ]; then
        echo "ERROR: Could not read serial runtime."
        echo "$serial_output"
        exit 1
    fi

    echo "Serial overall: $serial_overall s"


    # --------------------------------------------------------
    # MPI TASK 1
    # --------------------------------------------------------

    echo
    echo "Running MPI Task 1..."

    mpi_output=$(mpirun -np 4 "$MPI_EXEC" "$n")

    mpi_overall=$(echo "$mpi_output" |
        grep "Overall time" |
        awk '{print $NF}')

    if [ -z "$mpi_overall" ]; then
        echo "ERROR: Could not read MPI runtime."
        echo "$mpi_output"
        exit 1
    fi

    echo "MPI (4 processes) overall: $mpi_overall s"


    # --------------------------------------------------------
    # HYBRID CONFIGURATIONS
    # --------------------------------------------------------

    for config in "${configs[@]}"; do

        IFS=':' read -r mpi_processes omp_threads <<< "$config"

        total_threads=$((mpi_processes * omp_threads))

        echo
        echo "Running Hybrid: ${mpi_processes} MPI × ${omp_threads} OpenMP"
        echo "Total threads: $total_threads"

        hybrid_output=$(mpirun -np "$mpi_processes" \
            "$HYBRID_EXEC" "$n" "$omp_threads")

        hybrid_comp=$(echo "$hybrid_output" |
            grep "Computational time" |
            awk '{print $NF}')

        hybrid_overall=$(echo "$hybrid_output" |
            grep "Overall time" |
            awk '{print $NF}')

        if [ -z "$hybrid_overall" ]; then
            echo "ERROR: Could not read hybrid runtime."
            echo "$hybrid_output"
            exit 1
        fi

        # ----------------------------------------------------
        # SPEEDUPS
        # ----------------------------------------------------

        # Speedup against SERIAL
        serial_speedup=$(awk \
            -v s="$serial_overall" \
            -v h="$hybrid_overall" \
            'BEGIN { printf "%.6f", s/h }')

        # Speedup against MPI Task 1
        mpi_speedup=$(awk \
            -v m="$mpi_overall" \
            -v h="$hybrid_overall" \
            'BEGIN { printf "%.6f", m/h }')

        # ----------------------------------------------------
        # SAVE TO CSV
        # ----------------------------------------------------

        echo "$n,$mpi_processes,$omp_threads,$total_threads,$serial_overall,$mpi_overall,$hybrid_comp,$hybrid_overall,$serial_speedup,$mpi_speedup" >> "$RESULTS"

        echo "  Computational: $hybrid_comp s"
        echo "  Overall:       $hybrid_overall s"
        echo "  Serial speedup: $serial_speedup x"
        echo "  MPI speedup:    $mpi_speedup x"

        # ----------------------------------------------------
        # CLEAN OUTPUT FILE
        # ----------------------------------------------------

        rm -f "primes_mpi_${n}.txt"

    done

    echo

done

# ------------------------------------------------------------
# COMPLETE
# ------------------------------------------------------------

echo
echo "============================================================"
echo "ALL TASK 2 TESTS COMPLETE"
echo "============================================================"
echo
echo "Results saved to:"
echo "$RESULTS"
echo
echo "You can inspect the results with:"
echo "cat $RESULTS"