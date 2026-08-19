#!/bin/bash

# Compile the program
gcc Task2.c -o task2 -lm -pthread

# Test values
tests=(
    10000001
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

# CSV header
echo "N,POSIX Overall Time(s),POSIX Computational Time(s)" > test2.csv

# Run every test
for n in "${tests[@]}"; do

    echo "Running test for N=$n..."

    result=$(echo "$n" | ./task2)

    overall_time=$(echo "$result" |
        grep "Overall time" |
        awk '{print $NF}')

    computational_time=$(echo "$result" |
        grep "Computational time" |
        awk '{print $NF}')

    echo "$n,$overall_time,$computational_time" >> test2_2threads.csv

done

echo "All tests complete."
echo "Results saved to test2.csv"