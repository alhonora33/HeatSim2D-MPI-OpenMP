#!/bin/bash
#SBATCH --job-name=perf_seq           # Nom du job
#SBATCH --nodes=1                     # Nombre de nœuds
#SBATCH --ntasks=1                    # Nombre de processus MPI (1 pour un seul processus)
#SBATCH --cpus-per-task=1             # Nombre de threads OpenMP
#SBATCH --exclusive                   # Réservation exclusive du nœud
#SBATCH --time=01:00:00               # Temps limite du job (1 heure)
#SBATCH --output=stencil_seq_%j.out       # Fichier de sortie (%j sera remplacé par le job ID)
#SBATCH --error=stencil_seq_%j.err        # Fichier d’erreur

module purge
module load compiler/gcc/12.2.0 mpi/openmpi/4.1.5

# Répertoire de soumission
cd $SLURM_SUBMIT_DIR

# Compilation du code
make clean && make

# Segment 1 : 20 à 64 (10 répétitions, pas de 1)
for N in $(seq 20 1 64); do
    for i in {1..10}; do
        srun --cpu-bind=cores ./bin/stencil_seq $N
    done
done

# Segment 2 : 64 à 120 (1 répétition, pas de 1)
for N in $(seq 64 1 120); do
    srun --cpu-bind=cores ./bin/stencil_seq $N
done
