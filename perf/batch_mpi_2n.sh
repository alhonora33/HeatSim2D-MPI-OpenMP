#!/bin/bash
#SBATCH --job-name=perf_mpi_2nodes    # Nom du job
#SBATCH --nodes=2                     # Nombre de nœuds
#SBATCH --cpus-per-task=1             # Nombre de threads OpenMP
#SBATCH --exclusive                   # Réservation exclusive du nœud
#SBATCH --time=01:00:00               # Temps limite du job (1 heure)
#SBATCH --output=stencil_mpi_2nodes_%j.out  # Fichier de sortie
#SBATCH --error=stencil_mpi_2nodes_%j.err   # Fichier d’erreur

# Chargement des modules
module purge
module load compiler/gcc/12.2.0 mpi/openmpi/4.1.5

# Répertoire de soumission
cd $SLURM_SUBMIT_DIR

# Compilation du code
make clean && make

# Tests pour 2 nœuds
for tasks in 4 9 16 25 36; do
    size=$(( ($(echo "sqrt($tasks)" | bc) * 46) + 2 ))
    echo "Running test on 2 nodes with $tasks MPI tasks and size $size."
    srun --ntasks=$tasks --cpu-bind=cores ./bin/stencil_mpi $size
done

# Nettoyage après les tests
make clean

