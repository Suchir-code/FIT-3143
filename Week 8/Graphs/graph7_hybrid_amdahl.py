import pandas as pd
import matplotlib.pyplot as plt

hybrid = pd.read_csv("../Data/graph5_7_hybrid.csv")
serial = pd.read_csv("../Data/week4_serial_for_task2.csv")

N_VALUE = 300000000

# --------------------------------------------------
# Week 4 serial baseline for empirical speedup
# --------------------------------------------------

serial_row = serial[serial["n"] == N_VALUE].iloc[0]
serial_overall = serial_row["serial_overall"]

# --------------------------------------------------
# 1 MPI x 1 OpenMP thread timing for Amdahl fractions
# --------------------------------------------------

baseline = hybrid[
    (hybrid["mpi_processes"] == 1) &
    (hybrid["omp_threads"] == 1)
].iloc[0]

overall_1 = baseline["hybrid_overall"]
parallel_time = baseline["sieve_time"]

serial_time = overall_1 - parallel_time

S = serial_time / overall_1
P = parallel_time / overall_1

print("Serial time:", serial_time)
print("Parallel time:", parallel_time)
print("Serial fraction S:", S)
print("Parallel fraction P:", P)
print("S + P:", S + P)

# --------------------------------------------------
# Scaling configurations
# --------------------------------------------------

scaling = hybrid[
    (
        (hybrid["mpi_processes"] == 1) &
        (hybrid["omp_threads"] == 1)
    )
    |
    (
        hybrid["omp_threads"] == 2
    )
].copy()

scaling = scaling.sort_values("total_threads")

# Remove duplicate total worker count if necessary.
scaling = scaling.drop_duplicates(
    subset=["total_threads"],
    keep="first"
)

# Empirical speedup relative to Week 4 serial.
scaling["empirical_speedup"] = (
    serial_overall / scaling["hybrid_overall"]
)

# Amdahl theoretical speedup.
scaling["theoretical_speedup"] = (
    1 / (
        S +
        P / scaling["total_threads"]
    )
)

print()
print(scaling[
    [
        "mpi_processes",
        "omp_threads",
        "total_threads",
        "hybrid_overall",
        "empirical_speedup",
        "theoretical_speedup"
    ]
])

# --------------------------------------------------
# Plot
# --------------------------------------------------

plt.figure(figsize=(8, 5))

plt.plot(
    scaling["total_threads"],
    scaling["empirical_speedup"],
    marker="o",
    linewidth=2,
    label="Empirical Speedup"
)

plt.plot(
    scaling["total_threads"],
    scaling["theoretical_speedup"],
    marker="o",
    linewidth=2,
    label="Amdahl Theoretical Speedup"
)

plt.xlabel("Total MPI + OpenMP Workers")
plt.ylabel("Speedup")
plt.title(
    "Hybrid MPI + OpenMP: Empirical vs Theoretical Speedup\n"
    "at n = 300 Million"
)

plt.xticks([1, 2, 4, 8, 16])
plt.grid(True, alpha=0.3)
plt.legend()

plt.tight_layout()
plt.savefig("graph7_hybrid_amdahl.png", dpi=300)
plt.show()