#!/bin/bash

SERIAL_EXEC="../task1"
RESULTS="serial_results.csv"

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

echo "N,Serial_Computational,Serial_Overall" > "$RESULTS"

for n in "${tests[@]}"; do

    echo "========================================"
    echo "Testing N = $n"
    echo "========================================"

    echo "Running Serial Task 1..."

    serial_result=$(echo "$n" | "$SERIAL_EXEC")

    serial_comp=$(echo "$serial_result" |
        grep "Computational time" |
        awk '{print $NF}')

    serial_overall=$(echo "$serial_result" |
        grep "Overall time" |
        awk '{print $NF}')

    echo "Serial:"
    echo "  Computational = $serial_comp s"
    echo "  Overall       = $serial_overall s"

    if [ -z "$serial_comp" ] || [ -z "$serial_overall" ]; then
        echo "ERROR: Could not extract timing for N=$n"
        exit 1
    fi

    echo "$n,$serial_comp,$serial_overall" >> "$RESULTS"

    rm -f "primes_${n}.txt"

    echo

done

echo
echo "========================================"
echo "ALL SERIAL TESTS COMPLETE"
echo "========================================"
echo
echo "Results saved to:"
echo "$RESULTS"