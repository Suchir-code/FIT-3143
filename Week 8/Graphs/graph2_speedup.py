import pandas as pd
import matplotlib.pyplot as plt

# Speedup = Serial Overall Time / Parallel Overall Time
mpi = pd.read_csv("../task1_n_scaling.csv")
openmp = pd.read_csv("../task3_n_scaling.csv")

# Use the same serial baseline for both MPI and OpenMP
mpi_speedup = mpi["serial_overall"] / mpi["mpi_overall"]
openmp_speedup = mpi["serial_overall"] / openmp["openmp_overall"]

x_millions = mpi["n"] / 1_000_000

plt.figure(figsize=(10, 6))

plt.plot(
    x_millions,
    mpi_speedup,
    marker="o",
    label="OpenMPI - 8 Processes"
)

plt.plot(
    x_millions,
    openmp_speedup,
    marker="o",
    label="OpenMP - 8 Threads"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Empirical Speedup")
plt.title("OpenMPI vs OpenMP Empirical Speedup (8 Processes/Threads)")
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.savefig("graph2_speedup.png", dpi=300)
plt.show()