#!/bin/bash
#SBATCH --job-name=perf_hybrid_4nodes    # Nom du job
#SBATCH --nodes=4                     # Nombre de nœuds
#SBATCH --ntasks=16                   # Nombre de processus MPI (1 tâche par NUMA)
#SBATCH --cpus-per-task=6             # Nombre de threads OpenMP par tâche
#SBATCH --exclusive                   # Réservation exclusive des nœuds
#SBATCH --time=01:00:00               # Temps limite (1 heure)
#SBATCH --output=stencil_hybrid_4nodes_%j.out  # Fichier de sortie
#SBATCH --error=stencil_hybrid_4nodes_%j.err   # Fichier d’erreur

# Chargement des modules
module purge
module load compiler/gcc/12.2.0 mpi/openmpi/4.1.5

# Répertoire de soumission
cd $SLURM_SUBMIT_DIR

# Compilation du code
make clean && make

# Configuration OpenMP
export OMP_PLACES="cores"
export OMP_PROC_BIND="close"

# Taille du stencil fixe
STENCIL_SIZE=48

# Exécution du test
echo "Running with 16 MPI tasks, 6 OpenMP threads per task, and stencil size $STENCIL_SIZE."

for i in {1..10}; do
    srun --ntasks=16 --cpus-per-task=6 --cpu-bind=cores ./bin/stencil_hybrid $STENCIL_SIZE
done

# Nettoyage après les tests
make clean

