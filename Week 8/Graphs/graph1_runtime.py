import pandas as pd
import matplotlib.pyplot as plt

mpi = pd.read_csv("../task1_n_scaling.csv")
openmp = pd.read_csv("../task3_n_scaling.csv")

x_millions = mpi["n"] / 1_000_000

plt.figure(figsize=(10, 6))

plt.plot(
    x_millions,
    mpi["mpi_overall"],
    marker="o",
    label="OpenMPI - 8 Processes"
)

plt.plot(
    x_millions,
    openmp["openmp_overall"],
    marker="o",
    label="OpenMP - 8 Threads"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Overall Execution Time (seconds)")
plt.title("OpenMPI vs OpenMP Overall Execution Time (8 Processes/Threads)")
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.savefig("graph1_runtime.png", dpi=300)
plt.show()