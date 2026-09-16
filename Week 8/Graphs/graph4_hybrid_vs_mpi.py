import pandas as pd
import matplotlib.pyplot as plt

graph4 = pd.read_csv("../Data/graph4_hybrid_threads.csv")
serial = pd.read_csv("../Data/week4_serial_for_task2.csv")

N_VALUE = 300000000

serial_row = serial[serial["n"] == N_VALUE].iloc[0]
serial_overall = serial_row["serial_overall"]

graph4["mpi_empirical_speedup"] = (
    serial_overall / graph4["mpi_overall"]
)

graph4["hybrid_empirical_speedup"] = (
    serial_overall / graph4["hybrid_overall"]
)

plt.figure(figsize=(8, 5))

plt.plot(
    graph4["omp_threads"],
    graph4["mpi_empirical_speedup"],
    marker="o",
    linewidth=2,
    label="Task 1 OpenMPI"
)

plt.plot(
    graph4["omp_threads"],
    graph4["hybrid_empirical_speedup"],
    marker="o",
    linewidth=2,
    label="Hybrid MPI + OpenMP"
)

plt.xlabel("OpenMP Threads per MPI Process")
plt.ylabel("Empirical Speedup")
plt.title(
    "Hybrid MPI + OpenMP vs Task 1 OpenMPI Empirical Speedup\n"
    "at n = 300 Million"
)

plt.xticks([1, 2, 4, 8])
plt.grid(True, alpha=0.3)
plt.legend()

plt.tight_layout()
plt.savefig("graph4_hybrid_vs_mpi.png", dpi=300)
plt.show()