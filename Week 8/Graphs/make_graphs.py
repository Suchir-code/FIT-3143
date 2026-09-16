import pandas as pd
import matplotlib.pyplot as plt

graph6 = pd.read_csv(
    r"D:\FIT 3143 group stuff\FIT-3143\Week 8\Data\graph6_task1_theory_vs_empirical.csv"
)

graph7 = pd.read_csv(
    r"D:\FIT 3143 group stuff\FIT-3143\Week 8\Data\graph7_task2_theory_vs_empirical.csv"
)

plt.figure(figsize=(8, 5))
plt.plot(
    graph6["mpi_processes"],
    graph6["empirical_speedup"],
    marker="o",
    label="Empirical Speedup"
)
plt.plot(
    graph6["mpi_processes"],
    graph6["theoretical_speedup"],
    marker="s",
    label="Theoretical Speedup"
)
plt.xlabel("Number of MPI Processes")
plt.ylabel("Speedup")
plt.title("Graph 6: Task 1 Empirical vs Theoretical Speedup")
plt.xticks(graph6["mpi_processes"])
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("graph6_task1_empirical_vs_theoretical.png", dpi=300)
plt.show()

plt.figure(figsize=(8, 5))
plt.plot(
    graph7["total_workers"],
    graph7["empirical_speedup"],
    marker="o",
    label="Empirical Speedup"
)
plt.plot(
    graph7["total_workers"],
    graph7["theoretical_speedup"],
    marker="s",
    label="Theoretical Speedup"
)
plt.xlabel("Total Workers (MPI Processes × OpenMP Threads)")
plt.ylabel("Speedup")
plt.title("Graph 7: Task 2 Empirical vs Theoretical Speedup")
plt.xticks(graph7["total_workers"])
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("graph7_task2_empirical_vs_theoretical.png", dpi=300)
plt.show()