import re
import matplotlib.pyplot as plt
from collections import defaultdict

# Fichiers d'entrée
file_paths = [
    ("stencil_mpi_1node_1398628.out", 1),
    ("stencil_mpi_2nodes_1398633.out", 2),
    ("stencil_mpi_4nodes_1398636.out", 4),
]

# Structure de données pour stocker les résultats
results = defaultdict(lambda: {"tasks": [], "size": [], "steps": [], "time": [], "gflops": []})

# Lecture et analyse des fichiers
for file_path, nodes in file_paths:
    size, steps, time, gflops = 0, 0, 0, 0  # Initialisation des variables
    with open(file_path, "r") as file:
        current_tasks = None
        for line in file:
            line = line.strip()
            
            # Détecter le nombre de tâches MPI
            if line.startswith("Running test on"):
                match = re.search(r"Running test on (\d+) nodes? with (\d+) MPI tasks", line)
                if match:
                    current_tasks = int(match.group(2))

            # Extraire size
            elif line.startswith("# size ="):
                match = re.search(r"# size = (\d+)", line)
                if match:
                    size = int(match.group(1))

            # Extraire steps
            elif line.startswith("# steps ="):
                match = re.search(r"# steps = (\d+)", line)
                if match:
                    steps = int(match.group(1))

            # Extraire time
            elif line.startswith("# time ="):
                match = re.search(r"# time = ([\d\.e\+]+)", line)
                if match:
                    time = float(match.group(1))

            # Extraire gflops
            elif line.startswith("# gflops ="):
                match = re.search(r"# gflops = ([\d\.]+)", line)
                if match:
                    gflops = float(match.group(1))

                    # Ajouter les données
                    results[nodes]["tasks"].append(current_tasks)
                    results[nodes]["size"].append(size)
                    results[nodes]["steps"].append(steps)
                    results[nodes]["time"].append(time / 1_000_000)  # Conversion µs -> s
                    results[nodes]["gflops"].append(gflops)

# Préparation des données pour les graphiques
tasks_all = []
time_all = []
gflops_all = []
sizes_all = []
nodes_all = []

for nodes, data in results.items():
    tasks_all.extend(data["tasks"])
    time_all.extend(data["time"])
    gflops_all.extend(data["gflops"])
    sizes_all.extend(data["size"])
    nodes_all.extend([nodes] * len(data["tasks"]))

# Positionnement des barres
x = range(len(tasks_all))  # Positions sur l'axe des abscisses
bar_width = 0.4

# Création du graphique à barres
fig, ax1 = plt.subplots(figsize=(12, 6))

# Barre pour le temps (axe gauche)
bars_time = ax1.bar(
    [i - bar_width / 2 for i in x], time_all, bar_width, label="Temps et Taille", color='skyblue', edgecolor='blue'
)
ax1.set_ylabel("Temps d'exécution (s)", color="blue")
ax1.tick_params(axis='y', labelcolor="blue")

# Ajouter la taille au-dessus des barres de temps
for i, bar in enumerate(bars_time):
    ax1.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.1,
             f"{sizes_all[i]}", ha='center', va='bottom', fontsize=8, color='blue')

# Barre pour les GFLOPS (axe droit)
ax2 = ax1.twinx()
bars_gflops = ax2.bar(
    [i + bar_width / 2 for i in x], gflops_all, bar_width, label="GFLOPS et Node", color='lightcoral', edgecolor='red'
)
ax2.set_ylabel("GFLOPS", color="red")
ax2.tick_params(axis='y', labelcolor="red")

# Ajouter le nombre de nœuds au-dessus des barres de GFLOPS
for i, bar in enumerate(bars_gflops):
    ax2.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.1,
             f"{nodes_all[i]}", ha='center', va='bottom', fontsize=8, color='red')

# Configuration de l'axe des abscisses
plt.xticks(x, tasks_all, rotation=45)
ax1.set_xlabel("Nombre de tâches MPI")

# Titre et légende
plt.title("Comparaison du temps et des GFLOPS par configuration (nodes, tasks)")
fig.legend(loc="upper left", bbox_to_anchor=(0.1, 0.9))

# Grille et affichage
ax1.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()

