import pandas as pd
import matplotlib.pyplot as plt

hybrid = pd.read_csv("../Data/graph5_7_hybrid.csv")
openmp = pd.read_csv("../Data/graph5_openmp.csv")
serial = pd.read_csv("../Data/week4_serial_for_task2.csv")

N_VALUE = 300000000

serial_row = serial[serial["n"] == N_VALUE].iloc[0]
serial_overall = serial_row["serial_overall"]

hybrid = hybrid[
    hybrid["omp_threads"] == 2
].copy()

hybrid["empirical_speedup"] = (
    serial_overall / hybrid["hybrid_overall"]
)

openmp["empirical_speedup"] = (
    serial_overall / openmp["openmp_overall"]
)

plt.figure(figsize=(8, 5))

plt.plot(
    hybrid["total_threads"],
    hybrid["empirical_speedup"],
    marker="o",
    linewidth=2,
    label="Hybrid MPI + OpenMP"
)

plt.plot(
    openmp["openmp_threads"],
    openmp["empirical_speedup"],
    marker="o",
    linewidth=2,
    label="OpenMP"
)

plt.xlabel("Total Parallel Workers / Threads")
plt.ylabel("Empirical Speedup")
plt.title(
    "Hybrid MPI + OpenMP vs OpenMP Empirical Speedup\n"
    "at n = 300 Million"
)

plt.xticks([2, 4, 8, 16])
plt.grid(True, alpha=0.3)
plt.legend()

plt.tight_layout()
plt.savefig("graph5_hybrid_vs_openmp.png", dpi=300)
plt.show()