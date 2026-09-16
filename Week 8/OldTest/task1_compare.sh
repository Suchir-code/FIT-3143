#!/bin/bash

# -----------------------------------
# COMPILE SERIAL AND MPI TASK 1
# -----------------------------------

echo "Compiling Task 1 programs..."

gcc Task1.c -o task1 -lm

if [ $? -ne 0 ]; then
    echo "Serial Task 1 compilation failed."
    exit 1
fi

mpicc Task1_MPI.c -o task1_mpi -lm

if [ $? -ne 0 ]; then
    echo "MPI Task 1 compilation failed."
    exit 1
fi

echo "Compilation successful."
echo


# -----------------------------------
# TEST VALUES
# -----------------------------------

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
)


# -----------------------------------
# CSV HEADER
# -----------------------------------

echo "N,Serial Computational,Serial Overall,MPI1 Computational,MPI1 Overall,MPI1 Comp Speedup,MPI1 Overall Speedup,MPI2 Computational,MPI2 Overall,MPI2 Comp Speedup,MPI2 Overall Speedup,MPI4 Computational,MPI4 Overall,MPI4 Comp Speedup,MPI4 Overall Speedup,MPI8 Computational,MPI8 Overall,MPI8 Comp Speedup,MPI8 Overall Speedup" > task1_comparison.csv


# -----------------------------------
# RUN TESTS
# -----------------------------------

for n in "${tests[@]}"; do

    echo "========================================"
    echo "Testing N = $n"
    echo "========================================"


    # -----------------------------------
    # SERIAL TASK 1
    # -----------------------------------

    echo "Running Serial Task 1..."

    serial_result=$(echo "$n" | ./task1)

    serial_comp=$(echo "$serial_result" |
        grep "Computational time" |
        awk '{print $NF}')

    serial_overall=$(echo "$serial_result" |
        grep "Overall time" |
        awk '{print $NF}')


    echo "Serial:"
    echo "  Computational = $serial_comp s"
    echo "  Overall       = $serial_overall s"


    # -----------------------------------
    # MPI - 1 PROCESS
    # -----------------------------------

    echo "Running MPI with 1 process..."

    mpi1_result=$(
        mpirun --allow-run-as-root -np 1 ./task1_mpi "$n"
    )

    mpi1_comp=$(echo "$mpi1_result" |
        grep "Computational time" |
        awk '{print $NF}')

    mpi1_overall=$(echo "$mpi1_result" |
        grep "Overall time" |
        awk '{print $NF}')

    mpi1_comp_speedup=$(awk \
        -v s="$serial_comp" \
        -v p="$mpi1_comp" \
        'BEGIN { printf "%.6f", s/p }')

    mpi1_overall_speedup=$(awk \
        -v s="$serial_overall" \
        -v p="$mpi1_overall" \
        'BEGIN { printf "%.6f", s/p }')


    # -----------------------------------
    # MPI - 2 PROCESSES
    # -----------------------------------

    echo "Running MPI with 2 processes..."

    mpi2_result=$(
        mpirun --allow-run-as-root -np 2 ./task1_mpi "$n"
    )

    mpi2_comp=$(echo "$mpi2_result" |
        grep "Computational time" |
        awk '{print $NF}')

    mpi2_overall=$(echo "$mpi2_result" |
        grep "Overall time" |
        awk '{print $NF}')

    mpi2_comp_speedup=$(awk \
        -v s="$serial_comp" \
        -v p="$mpi2_comp" \
        'BEGIN { printf "%.6f", s/p }')

    mpi2_overall_speedup=$(awk \
        -v s="$serial_overall" \
        -v p="$mpi2_overall" \
        'BEGIN { printf "%.6f", s/p }')


    # -----------------------------------
    # MPI - 4 PROCESSES
    # -----------------------------------

    echo "Running MPI with 4 processes..."

    mpi4_result=$(
        mpirun --allow-run-as-root -np 4 ./task1_mpi "$n"
    )

    mpi4_comp=$(echo "$mpi4_result" |
        grep "Computational time" |
        awk '{print $NF}')

    mpi4_overall=$(echo "$mpi4_result" |
        grep "Overall time" |
        awk '{print $NF}')

    mpi4_comp_speedup=$(awk \
        -v s="$serial_comp" \
        -v p="$mpi4_comp" \
        'BEGIN { printf "%.6f", s/p }')

    mpi4_overall_speedup=$(awk \
        -v s="$serial_overall" \
        -v p="$mpi4_overall" \
        'BEGIN { printf "%.6f", s/p }')


    # -----------------------------------
    # MPI - 8 PROCESSES
    # -----------------------------------

    echo "Running MPI with 8 processes..."

    mpi8_result=$(
        mpirun --allow-run-as-root -np 8 ./task1_mpi "$n"
    )

    mpi8_comp=$(echo "$mpi8_result" |
        grep "Computational time" |
        awk '{print $NF}')

    mpi8_overall=$(echo "$mpi8_result" |
        grep "Overall time" |
        awk '{print $NF}')

    mpi8_comp_speedup=$(awk \
        -v s="$serial_comp" \
        -v p="$mpi8_comp" \
        'BEGIN { printf "%.6f", s/p }')

    mpi8_overall_speedup=$(awk \
        -v s="$serial_overall" \
        -v p="$mpi8_overall" \
        'BEGIN { printf "%.6f", s/p }')


    # -----------------------------------
    # SAVE RESULTS
    # -----------------------------------

    echo "$n,$serial_comp,$serial_overall,$mpi1_comp,$mpi1_overall,$mpi1_comp_speedup,$mpi1_overall_speedup,$mpi2_comp,$mpi2_overall,$mpi2_comp_speedup,$mpi2_overall_speedup,$mpi4_comp,$mpi4_overall,$mpi4_comp_speedup,$mpi4_overall_speedup,$mpi8_comp,$mpi8_overall,$mpi8_comp_speedup,$mpi8_overall_speedup" >> task1_comparison.csv


    # -----------------------------------
    # DISPLAY RESULTS
    # -----------------------------------

    echo

    echo "Serial"
    echo "  Computational: $serial_comp s"
    echo "  Overall:       $serial_overall s"

    echo

    echo "MPI 1 process"
    echo "  Computational: $mpi1_comp s"
    echo "  Overall:       $mpi1_overall s"
    echo "  Comp speedup:  ${mpi1_comp_speedup}x"
    echo "  Overall speedup: ${mpi1_overall_speedup}x"

    echo

    echo "MPI 2 processes"
    echo "  Computational: $mpi2_comp s"
    echo "  Overall:       $mpi2_overall s"
    echo "  Comp speedup:  ${mpi2_comp_speedup}x"
    echo "  Overall speedup: ${mpi2_overall_speedup}x"

    echo

    echo "MPI 4 processes"
    echo "  Computational: $mpi4_comp s"
    echo "  Overall:       $mpi4_overall s"
    echo "  Comp speedup:  ${mpi4_comp_speedup}x"
    echo "  Overall speedup: ${mpi4_overall_speedup}x"

    echo

    echo "MPI 8 processes"
    echo "  Computational: $mpi8_comp s"
    echo "  Overall:       $mpi8_overall s"
    echo "  Comp speedup:  ${mpi8_comp_speedup}x"
    echo "  Overall speedup: ${mpi8_overall_speedup}x"

    echo


    # -----------------------------------
    # DELETE GENERATED PRIME FILES
    # -----------------------------------

    rm -f "primes_${n}.txt"
    rm -f "primes_mpi_${n}.txt"

done


echo
echo "========================================"
echo "ALL TASK 1 TESTS COMPLETE"
echo "========================================"
echo
echo "Results saved to:"
echo "task1_comparison.csv"