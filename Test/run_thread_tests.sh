#!/bin/bash

N=100000000

threads=(
    1
    2
    4
    6
    8
    10
    12
    16
    20
    24
)

echo "Compiling Task 1 and Task 3..."

gcc Task1.c -o task1 -lm
gcc Task3.c -o task3 -fopenmp -lm

echo
echo "Running serial baseline..."

result1=$(echo "$N" | ./task1)

serial_comp=$(echo "$result1" |
    grep "Computational time" |
    awk '{print $NF}')

serial_overall=$(echo "$result1" |
    grep "Overall time" |
    awk '{print $NF}')

echo "Serial computational time: $serial_comp s"
echo

echo "Threads,Serial Computational,POSIX Computational,POSIX Speedup,OpenMP Computational,OpenMP Speedup" > thread_results.csv


for t in "${threads[@]}"; do

    echo "========================================"
    echo "Testing $t threads"
    echo "========================================"


    # --------------------
    # TASK 2 - POSIX
    # --------------------

    gcc Task2.c -o task2 -pthread -lm -DNUM_THREADS=$t

    result2=$(echo "$N" | ./task2)

    task2_comp=$(echo "$result2" |
        grep "Computational time" |
        awk '{print $NF}')

    posix_speedup=$(awk -v s="$serial_comp" -v p="$task2_comp" \
        'BEGIN {printf "%.6f", s/p}')


    # --------------------
    # TASK 3 - OPENMP
    # --------------------

    result3=$(echo "$N" |
        OMP_NUM_THREADS=$t ./task3)

    task3_comp=$(echo "$result3" |
        grep "Computational time" |
        awk '{print $NF}')

    openmp_speedup=$(awk -v s="$serial_comp" -v p="$task3_comp" \
        'BEGIN {printf "%.6f", s/p}')


    # --------------------
    # SAVE
    # --------------------

    echo "$t,$serial_comp,$task2_comp,$posix_speedup,$task3_comp,$openmp_speedup" >> thread_results.csv


    echo "POSIX:  $task2_comp s | Speedup = ${posix_speedup}x"
    echo "OpenMP: $task3_comp s | Speedup = ${openmp_speedup}x"
    echo


    rm -f "primes2_${N}.txt"
    rm -f "primes3_${N}.txt"

done


rm -f "primes_${N}.txt"

echo
echo "========================================"
echo "THREAD TEST COMPLETE"
echo "========================================"
echo "Results saved to thread_results.csv"