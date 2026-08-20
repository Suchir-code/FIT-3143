import pandas as pd
import matplotlib.pyplot as plt

n_data = pd.read_csv("all_results.csv")
thread_data = pd.read_csv("thread_results.csv")

n_data["N Millions"] = n_data["N"] / 1_000_000
# Show n in millions to make the graphs easier to read
n_data["N Millions"] = n_data["N"] / 1_000_000


# ============================================================
# GRAPH 1
# Serial vs POSIX runtime with increasing n
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    n_data["N Millions"],
    n_data["Task1 Computational"],
    marker="o",
    label="Serial"
)

plt.plot(
    n_data["N Millions"],
    n_data["Task2 Computational"],
    marker="o",
    label="POSIX Threads"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Computational Time (s)")
plt.title("Serial vs POSIX Computational Time with Increasing n")
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph1_serial_vs_posix_n.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 2
# POSIX speed-up with increasing n
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    n_data["N Millions"],
    n_data["POSIX Speedup"],
    marker="o"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Speed-up (×)")
plt.title("POSIX Speed-up with Increasing n")
plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph2_posix_speedup_n.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 3
# Serial vs POSIX runtime with increasing thread count
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    thread_data["Threads"],
    thread_data["Serial Computational"],
    marker="o",
    label="Serial"
)

plt.plot(
    thread_data["Threads"],
    thread_data["POSIX Computational"],
    marker="o",
    label="POSIX Threads"
)

plt.xlabel("Number of Threads")
plt.ylabel("Computational Time (s)")
plt.title("Serial vs POSIX Computational Time with Increasing Threads")

plt.xticks(thread_data["Threads"])

plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph3_serial_vs_posix_threads.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 4
# POSIX speed-up with increasing thread count
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    thread_data["Threads"],
    thread_data["POSIX Speedup"],
    marker="o"
)

plt.xlabel("Number of Threads")
plt.ylabel("Speed-up (×)")
plt.title("POSIX Speed-up with Increasing Threads")

plt.xticks(thread_data["Threads"])

plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph4_posix_speedup_threads.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 5
# Serial vs OpenMP runtime with increasing n
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    n_data["N Millions"],
    n_data["Task1 Computational"],
    marker="o",
    label="Serial"
)

plt.plot(
    n_data["N Millions"],
    n_data["Task3 Computational"],
    marker="o",
    label="OpenMP"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Computational Time (s)")
plt.title("Serial vs OpenMP Computational Time with Increasing n")

plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph5_serial_vs_openmp_n.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 6
# OpenMP speed-up with increasing n
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    n_data["N Millions"],
    n_data["OpenMP Speedup"],
    marker="o"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Speed-up (×)")
plt.title("OpenMP Speed-up with Increasing n")

plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph6_openmp_speedup_n.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 7
# POSIX vs OpenMP runtime with increasing n
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    n_data["N Millions"],
    n_data["Task2 Computational"],
    marker="o",
    label="POSIX Threads"
)

plt.plot(
    n_data["N Millions"],
    n_data["Task3 Computational"],
    marker="o",
    label="OpenMP"
)

plt.xlabel("Input Size, n (Millions)")
plt.ylabel("Computational Time (s)")
plt.title("POSIX vs OpenMP Computational Time with Increasing n")

plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph7_posix_vs_openmp_n.png", dpi=300)
plt.close()


# ============================================================
# GRAPH 8
# POSIX vs OpenMP runtime with increasing thread count
# ============================================================

plt.figure(figsize=(8, 5))

plt.plot(
    thread_data["Threads"],
    thread_data["POSIX Computational"],
    marker="o",
    label="POSIX Threads"
)

plt.plot(
    thread_data["Threads"],
    thread_data["OpenMP Computational"],
    marker="o",
    label="OpenMP"
)

plt.xlabel("Number of Threads")
plt.ylabel("Computational Time (s)")
plt.title("POSIX vs OpenMP Computational Time with Increasing Threads")

plt.xticks(thread_data["Threads"])

plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()

plt.savefig("graph8_posix_vs_openmp_threads.png", dpi=300)
plt.close()


print("All 8 graphs generated successfully.")