#!/bin/bash

# ============================================================
# TASK 2 - MPI + OPENMP AUTOMATED PERFORMANCE TEST
# ============================================================

# ------------------------------------------------------------
# SETTINGS
# ------------------------------------------------------------

MPI_EXEC="./task1_mpi"
HYBRID_EXEC="./task2_hybrid"

RESULTS="task2_results3.csv"

# MPI baseline used for Graph 4
MPI_PROCESSES=4

# ------------------------------------------------------------
# N VALUES
# Existing serial measurements: 50M - 340M
# ------------------------------------------------------------

tests=(
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
    310000000
    320000000
    330000000
    340000000
    350000000
    360000000
    370000000
    380000000
)

# ------------------------------------------------------------
# EXISTING SERIAL OVERALL TIMES
#
# These were measured previously and are being reused as the
# serial baseline for Task 2.
# ------------------------------------------------------------

declare -A SERIAL_OVERALL

SERIAL_OVERALL[100000000]=2.137799
SERIAL_OVERALL[110000000]=2.327720
SERIAL_OVERALL[120000000]=2.662324
SERIAL_OVERALL[130000000]=2.828227
SERIAL_OVERALL[140000000]=3.068335
SERIAL_OVERALL[150000000]=3.365602
SERIAL_OVERALL[160000000]=3.552484
SERIAL_OVERALL[170000000]=3.680144
SERIAL_OVERALL[180000000]=4.057366
SERIAL_OVERALL[190000000]=4.423531
SERIAL_OVERALL[200000000]=4.493130
SERIAL_OVERALL[210000000]=4.895807
SERIAL_OVERALL[220000000]=5.163711
SERIAL_OVERALL[230000000]=5.582083
SERIAL_OVERALL[240000000]=5.758236
SERIAL_OVERALL[250000000]=5.874211
SERIAL_OVERALL[260000000]=6.273753
SERIAL_OVERALL[270000000]=6.888971
SERIAL_OVERALL[280000000]=6.606389
SERIAL_OVERALL[290000000]=7.794150
SERIAL_OVERALL[300000000]=7.591005
SERIAL_OVERALL[310000000]=8.606820
SERIAL_OVERALL[320000000]=9.152167
SERIAL_OVERALL[330000000]=10.072646
SERIAL_OVERALL[340000000]=9.756816
SERIAL_OVERALL[350000000]=18.877864
SERIAL_OVERALL[360000000]=11.162981
SERIAL_OVERALL[370000000]=10.567124
SERIAL_OVERALL[380000000]=12.447957

# ------------------------------------------------------------
# HYBRID CONFIGURATIONS
#
# MPI processes : OpenMP threads
#
# The first four are especially important for Graph 4:
# fixed 4 MPI processes, increasing OpenMP threads.
# ------------------------------------------------------------

configs=(
    "4:1"
    "4:2"
    "4:4"
    "4:8"
)

# ------------------------------------------------------------
# CHECK EXECUTABLES
# ------------------------------------------------------------

echo "Checking executables..."

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
    # GET EXISTING SERIAL BASELINE
    # --------------------------------------------------------

    serial_overall="${SERIAL_OVERALL[$n]}"

    if [ -z "$serial_overall" ]; then
        echo "ERROR: No serial baseline found for N=$n"
        exit 1
    fi

    echo "Existing serial overall: $serial_overall s"

    # --------------------------------------------------------
    # MPI TASK 1 BASELINE
    # --------------------------------------------------------

    echo
    echo "Running MPI Task 1 ($MPI_PROCESSES processes)..."

    mpi_output=$(mpirun -np "$MPI_PROCESSES" "$MPI_EXEC" "$n")

    mpi_overall=$(echo "$mpi_output" |
        grep "Overall time" |
        awk '{print $NF}')

    if [ -z "$mpi_overall" ]; then
        echo "ERROR: Could not read MPI runtime."
        echo "$mpi_output"
        exit 1
    fi

    echo "MPI overall: $mpi_overall s"

    # --------------------------------------------------------
    # HYBRID CONFIGURATIONS
    # --------------------------------------------------------

    for config in "${configs[@]}"; do

        IFS=':' read -r mpi_processes omp_threads <<< "$config"

        total_threads=$((mpi_processes * omp_threads))

        echo
        echo "Running Hybrid: ${mpi_processes} MPI × ${omp_threads} OpenMP"
        echo "Total execution threads: $total_threads"

        # --oversubscribe allows intentional testing of more
        # software threads than available CPU cores.
        hybrid_output=$(mpirun --oversubscribe \
            -np "$mpi_processes" \
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

        # Overall speedup compared with existing serial baseline
        serial_speedup=$(awk \
            -v s="$serial_overall" \
            -v h="$hybrid_overall" \
            'BEGIN { printf "%.6f", s/h }')

        # Overall speedup compared with MPI Task 1
        mpi_speedup=$(awk \
            -v m="$mpi_overall" \
            -v h="$hybrid_overall" \
            'BEGIN { printf "%.6f", m/h }')

        # ----------------------------------------------------
        # SAVE RESULTS
        # ----------------------------------------------------

        echo "$n,$mpi_processes,$omp_threads,$total_threads,$serial_overall,$mpi_overall,$hybrid_comp,$hybrid_overall,$serial_speedup,$mpi_speedup" >> "$RESULTS"

        echo "  Computational: $hybrid_comp s"
        echo "  Overall:       $hybrid_overall s"
        echo "  Serial speedup: $serial_speedup x"
        echo "  MPI speedup:    $mpi_speedup x"

        # ----------------------------------------------------
        # REMOVE OUTPUT FILE
        # ----------------------------------------------------

        rm -f "primes_mpi_${n}.txt"

    done

    echo

done

# ------------------------------------------------------------
# COMPLETE
# ------------------------------------------------------------

echo "============================================================"
echo "ALL TASK 2 TESTS COMPLETE"
echo "============================================================"
echo
echo "Results saved to:"
echo "$RESULTS"
echo
echo "Number of data rows:"
tail -n +2 "$RESULTS" | wc -l
echo
echo "You can inspect the results with:"
echo "cat $RESULTS"