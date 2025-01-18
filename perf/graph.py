import re
import matplotlib.pyplot as plt
from collections import defaultdict

# Limites des caches (calculées précédemment)
N_48 = 48  # Valeur optimale
N_L1 = 60  # Limite pour le cache L1
N_109 = 109  # convergence par step

# Fichier de test
file_path = "stencil_seq_1395628.out"

# Structure de données pour stocker les résultats
results = defaultdict(lambda: {"steps": [], "time": [], "gflops": []})

# Lire et analyser le fichier
with open(file_path, "r") as file:
    buffer = {}  # Stocker temporairement les informations extraites
    for line in file:
        line = line.strip()

        # Détecter le début d'un groupe
        if line.startswith("# init:"):
            buffer.clear()  # Réinitialiser le buffer pour un nouveau groupe

        # Extraire size
        elif line.startswith("# size ="):
            match = re.search(r"# size = (\d+)", line)
            if match:
                buffer["size"] = int(match.group(1))

        # Extraire steps
        elif line.startswith("# steps ="):
            match = re.search(r"# steps = (\d+)", line)
            if match:
                buffer["steps"] = int(match.group(1))

        # Extraire time
        elif line.startswith("# time ="):
            match = re.search(r"# time = ([\d\.e\+]+)", line)
            if match:
                buffer["time"] = float(match.group(1))

        # Extraire gflops
        elif line.startswith("# gflops ="):
            match = re.search(r"# gflops = ([\d\.]+)", line)
            if match:
                buffer["gflops"] = float(match.group(1))

                # Ajouter les données une fois le groupe complet
                if "size" in buffer and "steps" in buffer and "time" in buffer and "gflops" in buffer:
                    size = buffer["size"]
                    results[size]["steps"].append(buffer["steps"])
                    results[size]["time"].append(buffer["time"])
                    results[size]["gflops"].append(buffer["gflops"])

# Calculer les moyennes
average_results = {
    N: {
        "steps": sum(data["steps"]) / len(data["steps"]),
        "time": sum(data["time"]) / len(data["time"]),
        "gflops": sum(data["gflops"]) / len(data["gflops"]),
    }
    for N, data in results.items()
}

# Préparer les données pour les graphiques
Ns = sorted(average_results.keys())
steps_avg = [average_results[N]["steps"] for N in Ns]
gflops_avg = [average_results[N]["gflops"] for N in Ns]
time_avg = [average_results[N]["time"] / 1_000_000 for N in Ns]  # Conversion de µs à s

# Graphique combiné pour le temps et les steps
fig, ax1 = plt.subplots()

# Axe pour le temps (à gauche)
color = 'tab:blue'
ax1.set_xlabel('N (taille)')
ax1.set_ylabel('Temps (s)', color=color)  # Mise à jour de l'unité en secondes
ax1.plot(Ns, time_avg, label="Temps (s)", color=color)
ax1.tick_params(axis='y', labelcolor=color)
ax1.grid()

# Axe pour les steps (à droite)
ax2 = ax1.twinx()
color = 'tab:orange'
ax2.set_ylabel('Nombre d\'étapes', color=color)
ax2.plot(Ns, steps_avg, label="Steps", color=color)
ax2.tick_params(axis='y', labelcolor=color)

# Ajoute la limite convergence
plt.axvline(N_109, color="purple", linestyle="--", label="N=109 : Convergence")

# Ajoure la limite L1
plt.axvline(N_L1, color="red", linestyle="--", label="N=60 : Limite L1")

# Ajoute la valeur optimale
plt.axvline(N_48, color="green", linestyle="--", label="N=48 : Valeur optimale")

# Titre du graphique
plt.title("Temps (s) et Nombre d'étapes moyens en fonction de N")

# Légendes combinées avec une position personnalisée et un décalage de 0.1 
fig.legend(loc="upper left", bbox_to_anchor=(0.2, 0.85))

# Afficher le graphique
plt.show()

# Afficher le graphique
plt.show()# Graphique pour les GFLOPS
plt.figure()
plt.plot(Ns, gflops_avg, label="GFLOPS")
plt.xlabel("N (taille)")
plt.ylabel("GFLOPS")
plt.title("GFLOPS moyen en fonction de N")
plt.grid()
plt.legend()

# Ajouter la limite du cache L1
plt.axvline(N_L1, color="red", linestyle="--", label="N=60 : Limite L1")

# Ajoute la valeur optimale
plt.axvline(N_48, color="green", linestyle="--", label="N=48 : Valeur optimale")

# Ajoute la limite convergence
plt.axvline(N_109, color="purple", linestyle="--", label="N=109 : Convergence")

# Ajouter une légende pour les limites
plt.legend()
plt.show()

