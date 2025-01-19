#!/bin/bash
#SBATCH --job-name=perf_omp           # Nom du job
#SBATCH --nodes=1                     # Nombre de nœuds
#SBATCH --ntasks=1                    # Nombre de processus MPI
#SBATCH --cpus-per-task=24            # Nombre total de cœurs disponibles pour OpenMP
#SBATCH --exclusive                   # Réservation exclusive du nœud
#SBATCH --time=01:00:00               # Temps limite du job (1 heure)
#SBATCH --output=stencil_omp_%j.out   # Fichier de sortie (%j sera remplacé par le job ID)
#SBATCH --error=stencil_omp_%j.err    # Fichier d’erreur

module purge
module load compiler/gcc/12.2.0 mpi/openmpi/4.1.5

# Répertoire de soumission
cd $SLURM_SUBMIT_DIR

# Compilation du code
make clean && make


# Configurations de threads à tester
THREAD_CONFIGS=(1 6 12 18 24)

# Teste les configurations de threads
for THREADS in "${THREAD_CONFIGS[@]}"; do
    export OMP_NUM_THREADS=$THREADS
    export OMP_PLACES="cores"
    export OMP_PROC_BIND="close"

    
    echo "Running with $THREADS OpenMP threads and stencil size 48"

    # Exécuter avec la taille calculée
    for i in {1..10}; do
        srun --cpu-bind=cores bin/stencil_omp 48
    done
done

# Nettoyage après les tests
make clean

