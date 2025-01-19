import re
import matplotlib.pyplot as plt
from collections import defaultdict

# Fichier de test
file_path = "stencil_omp_1398822.out"

# Structure de données pour stocker les résultats
results = defaultdict(lambda: {"time": [], "gflops": []})

# Lire et analyser le fichier
with open(file_path, "r") as file:
    current_threads = 0
    for line in file:
        line = line.strip()

        # Détecter le nombre de threads
        if line.startswith("Running with"):
            match = re.search(r"Running with (\d+) OpenMP threads", line)
            if match:
                current_threads = int(match.group(1))

        # Extraire time
        elif line.startswith("# time ="):
            match = re.search(r"# time = ([\d\.e\+]+)", line)
            if match:
                time = float(match.group(1))
                results[current_threads]["time"].append(time)

        # Extraire gflops
        elif line.startswith("# gflops ="):
            match = re.search(r"# gflops = ([\d\.]+)", line)
            if match:
                gflops = float(match.group(1))
                results[current_threads]["gflops"].append(gflops)

# Calculer les moyennes pour chaque configuration
average_results = {
    threads: {
        "time": sum(data["time"]) / len(data["time"]),
        "gflops": sum(data["gflops"]) / len(data["gflops"]),
    }
    for threads, data in results.items()
}

# Préparer les données pour le graphique
threads = sorted(average_results.keys())
time_avg = [average_results[thread]["time"] / 1_000_000 for thread in threads]  # Conversion µs -> s
gflops_avg = [average_results[thread]["gflops"] for thread in threads]

# Calculer le nombre de NUMA utilisés
numa_used = [thread // 6 for thread in threads]

# Position des barres
x = range(len(threads))
bar_width = 0.4

# Création du graphique
fig, ax1 = plt.subplots(figsize=(10, 6))

# Barres pour le temps (axe gauche)
bars_time = ax1.bar(
    [i - bar_width / 2 for i in x], time_avg, bar_width, label="Temps (s)", color="skyblue", edgecolor="blue"
)
ax1.set_ylabel("Temps d'exécution (s)", color="blue")
ax1.tick_params(axis="y", labelcolor="blue")

# Ajouter le nombre de NUMA utilisés au-dessus des barres de temps
for i, bar in enumerate(bars_time):
    ax1.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.05,
             f"{numa_used[i]} NUMA", ha="center", va="bottom", fontsize=8, color="blue")

# Barres pour les GFLOPS (axe droit)
ax2 = ax1.twinx()
bars_gflops = ax2.bar(
    [i + bar_width / 2 for i in x], gflops_avg, bar_width, label="GFLOPS", color="lightcoral", edgecolor="red"
)
ax2.set_ylabel("GFLOPS", color="red")
ax2.tick_params(axis="y", labelcolor="red")

# Ajouter le nombre de NUMA utilisés au-dessus des barres de GFLOPS
for i, bar in enumerate(bars_gflops):
    ax2.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.05,
             f"{numa_used[i]} NUMA", ha="center", va="bottom", fontsize=8, color="red")

# Configuration de l'axe des abscisses
plt.xticks(x, threads)
ax1.set_xlabel("Nombre de threads OpenMP")

# Titre et légende
plt.title("Temps d'exécution et GFLOPS en fonction du nombre de threads et des NUMA utilisés")
fig.legend(loc="upper left", bbox_to_anchor=(0.1, 0.9))

# Grille et affichage
ax1.grid(axis="y", linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()

