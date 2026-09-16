import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("../task1_amdahl.csv")

plt.figure(figsize=(10, 6))

plt.plot(
    data["mpi_processes"],
    data["empirical_speedup"],
    marker="o",
    label="Empirical Speedup"
)

plt.plot(
    data["mpi_processes"],
    data["theoretical_speedup"],
    marker="o",
    label="Theoretical Speedup (Amdahl's Law)"
)

plt.xlabel("Number of MPI Processes")
plt.ylabel("Speedup")
plt.title("Task 1 OpenMPI: Empirical vs Theoretical Speedup at n = 300 Million")

plt.xticks([1, 2, 4, 8, 16])

plt.legend()
plt.grid(True)

plt.tight_layout()
plt.savefig("graph6_amdahl.png", dpi=300)
plt.show()