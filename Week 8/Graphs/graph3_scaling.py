import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("../graph3_300m.csv")

serial_overall = data.loc[
    data["type"] == "serial",
    "overall_time"
].iloc[0]

mpi = data[data["type"] == "mpi"]
openmp = data[data["type"] == "openmp"]

mpi_speedup = serial_overall / mpi["overall_time"]
openmp_speedup = serial_overall / openmp["overall_time"]

plt.figure(figsize=(10, 6))

plt.plot(
    mpi["count"],
    mpi_speedup,
    marker="o",
    label="OpenMPI"
)

plt.plot(
    openmp["count"],
    openmp_speedup,
    marker="o",
    label="OpenMP"
)

plt.xlabel("Number of Processes / Threads")
plt.ylabel("Empirical Speedup")
plt.title("OpenMPI vs OpenMP Empirical Speedup at n = 300 Million")

plt.xticks([1, 2, 4, 8, 16])

plt.legend()
plt.grid(True)

plt.tight_layout()
plt.savefig("graph3_scaling.png", dpi=300)
plt.show()