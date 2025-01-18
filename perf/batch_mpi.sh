#!/bin/bash
#SBATCH --job-name=perf_mpi           # Nom du job
#SBATCH --nodes=1                     # Nombre de nœuds
#SBATCH --ntasks=4                    # Nombre de processus MPI (1 pour un seul processus)
#SBATCH --cpus-per-task=1             # Nombre de threads OpenMP
#SBATCH --exclusive                   # Réservation exclusive du nœud
#SBATCH --time=01:00:00               # Temps limite du job (1 heure)
#SBATCH --output=stencil_mpi_%j.out       # Fichier de sortie (%j sera remplacé par le job ID)
#SBATCH --error=stencil_mpi_%j.err        # Fichier d’erreur

module purge
module load compiler/gcc/12.2.0 mpi/openmpi/4.1.5

# Répertoire de soumission
cd $SLURM_SUBMIT_DIR

# Compilation du code
make clean && make

srun ./bin/stencil_mpi 20 -t


