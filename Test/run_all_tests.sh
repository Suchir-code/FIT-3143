#!/bin/bash

# -----------------------------
# SETTINGS
# -----------------------------

OMP_THREADS=10

# -----------------------------
# COMPILE ALL TASKS
# -----------------------------

echo "Compiling programs..."

gcc Task1.c -o task1 -lm

gcc Task2.c -o task2 -pthread -lm

gcc Task3.c -o task3 -fopenmp -lm

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

echo "Compilation successful."
echo


# -----------------------------
# 30 DIFFERENT VALUES OF n
# -----------------------------

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


# -----------------------------
# CSV HEADER
# -----------------------------

echo "N,Task1 Computational,Task1 Overall,Task2 Computational,Task2 Overall,Task3 Computational,Task3 Overall,POSIX Speedup,OpenMP Speedup" > all_results.csv


# -----------------------------
# RUN ALL TESTS
# -----------------------------

for n in "${tests[@]}"; do

    echo "========================================"
    echo "Testing N = $n"
    echo "========================================"


    # -------------------------
    # TASK 1
    # -------------------------

    echo "Running Task 1..."

    result1=$(echo "$n" | ./task1)

    task1_comp=$(echo "$result1" |
        grep "Computational time" |
        awk '{print $NF}')

    task1_overall=$(echo "$result1" |
        grep "Overall time" |
        awk '{print $NF}')


    # -------------------------
    # TASK 2
    # -------------------------

    echo "Running Task 2..."

    result2=$(echo "$n" | ./task2)

    task2_comp=$(echo "$result2" |
        grep "Computational time" |
        awk '{print $NF}')

    task2_overall=$(echo "$result2" |
        grep "Overall time" |
        awk '{print $NF}')


    # -------------------------
    # TASK 3
    # -------------------------

    echo "Running Task 3 with $OMP_THREADS threads..."

    result3=$(echo "$n" |
        OMP_NUM_THREADS=$OMP_THREADS ./task3)

    task3_comp=$(echo "$result3" |
        grep "Computational time" |
        awk '{print $NF}')

    task3_overall=$(echo "$result3" |
        grep "Overall time" |
        awk '{print $NF}')


    # -------------------------
    # CALCULATE SPEEDUPS
    # -------------------------

    posix_speedup=$(awk -v s="$task1_comp" -v p="$task2_comp" \
        'BEGIN { printf "%.6f", s/p }')

    openmp_speedup=$(awk -v s="$task1_comp" -v p="$task3_comp" \
        'BEGIN { printf "%.6f", s/p }')


    # -------------------------
    # SAVE RESULT
    # -------------------------

    echo "$n,$task1_comp,$task1_overall,$task2_comp,$task2_overall,$task3_comp,$task3_overall,$posix_speedup,$openmp_speedup" >> all_results.csv


    echo "Task 1: $task1_comp s"
    echo "Task 2: $task2_comp s | Speedup = ${posix_speedup}x"
    echo "Task 3: $task3_comp s | Speedup = ${openmp_speedup}x"
    echo


    # -------------------------
    # DELETE LARGE PRIME FILES
    # -------------------------

    rm -f "primes_${n}.txt"
    rm -f "primes2_${n}.txt"
    rm -f "primes3_${n}.txt"

done


echo
echo "========================================"
echo "ALL TESTS COMPLETE"
echo "========================================"
echo
echo "Results saved to:"
echo "all_results.csv"