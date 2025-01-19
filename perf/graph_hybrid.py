import matplotlib.pyplot as plt
import numpy as np

# Données extraites des résultats
results = {
    6: {"time": [6.6429e+06, 2.8353e+06, 2.20193e+06, 2.17997e+06, 2.12776e+06, 
                 2.10851e+06, 2.11929e+06, 2.12596e+06, 2.20853e+06, 2.1037e+06],
        "gflops": [0.0381368, 0.0893517, 0.115053, 0.116212, 0.119064, 
                   0.120151, 0.119539, 0.119164, 0.114709, 0.120425]},
    12: {"time": [2.47389e+07, 3.57305e+06, 4.86027e+06, 2.95894e+06, 2.92277e+06, 
                  2.88007e+06, 3.0068e+06, 2.97031e+06, 2.85928e+06, 2.89726e+06],
         "gflops": [0.0102405, 0.0709026, 0.0521244, 0.0856181, 0.0866776, 
                    0.0879628, 0.0842552, 0.0852902, 0.0886023, 0.0874408]}
}

# Calcul des moyennes
avg_results = {
    threads: {
        "time": np.mean(data["time"]) / 1e+06,  # Conversion en secondes
        "gflops": np.mean(data["gflops"])
    }
    for threads, data in results.items()
}

# Préparer les données pour le graphique
threads = list(avg_results.keys())
time_avg = [avg_results[t]["time"] for t in threads]
gflops_avg = [avg_results[t]["gflops"] for t in threads]

# Position des barres
x = np.arange(len(threads))
bar_width = 0.4

# Création du graphique
fig, ax1 = plt.subplots(figsize=(10, 6))

# Barres pour le temps d'exécution
bars_time = ax1.bar(
    x - bar_width / 2, time_avg, bar_width, label="Temps (s)", color="skyblue", edgecolor="blue"
)
ax1.set_ylabel("Temps d'exécution (s)", color="blue")
ax1.tick_params(axis="y", labelcolor="blue")

# Barres pour les GFLOPS
ax2 = ax1.twinx()
bars_gflops = ax2.bar(
    x + bar_width / 2, gflops_avg, bar_width, label="GFLOPS", color="lightcoral", edgecolor="red"
)
ax2.set_ylabel("GFLOPS", color="red")
ax2.tick_params(axis="y", labelcolor="red")

# Ajouter des étiquettes au-dessus des barres
for i, bar in enumerate(bars_time):
    ax1.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.05,
             f"{threads[i]} threads", ha="center", va="bottom", fontsize=8, color="blue")
# Configuration de l'axe des abscisses
plt.xticks(x, threads)
ax1.set_xlabel("Nombre de threads OpenMP")

# Titre et légende
plt.title("Temps d'exécution et GFLOPS en fonction du nombre de threads OpenMP")
fig.legend(loc="upper left", bbox_to_anchor=(0.1, 0.9))

# Grille et affichage
ax1.grid(axis="y", linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()

