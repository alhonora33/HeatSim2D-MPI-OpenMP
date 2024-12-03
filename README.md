# HeatSim2D-MPI-OpenMP

This project simulates heat transmission in a 2D solid using numerical methods.  
Dirichlet boundary conditions are applied, and iterations continue until a steady-state is reached (using the 𝜖 convergence criterion).  
OpenMP is employed for shared memory parallelism, while MPI distributes the workload across nodes, ensuring efficient computation for large-scale problems.

---

## Architecture Overview

### Node

- **Processor**: Intel(R) Xeon(R) CPU E5-2680 v3
- **Total Physical Processors**: 2
- **Nominal Frequency**: 2.50 GHz

### Core Configuration

- **Number of Physical Cores per Processor**: 12
- **Total Physical Cores**: 24 (12 per processor)

### NUMA

Each processor is divided into two NUMA nodes, for a total of 4 NUMA nodes:

- **Processor 0**: NUMA Node 0 (6 cores) and NUMA Node 1 (6 cores)
- **Processor 1**: NUMA Node 2 (6 cores) and NUMA Node 3 (6 cores)

- **Memory Configuration**:
  - Each NUMA node has 32 GB of local memory.
  - **Total memory**: 128 GB (32 GB x 4 nodes).

### Cache Configuration

#### **L1 Cache**:

- 32 KB (data) + 32 KB (instructions) per core
- **Total for 24 cores**: 1.5 MB

#### **L2 Cache**:

- 256 KB per core
- **Total for 24 cores**: 6 MB

#### **L3 Cache**:

- 15 MB shared among 6 cores per NUMA node
- **Total for 4 NUMA nodes**: 60 MB

---

## Step 0: Compilation and Scenario Setup

### 0.1: Compilation

To compile the code, run the following command:

```bash
make
```

Tree structure:

```plaintext
.
├── bin
│   ├── stencil_hybrid
│   ├── stencil_mpi
│   ├── stencil_omp
│   └── stencil_seq
├── build
│   ├── stencil_hybrid.o
│   ├── stencil_mpi.o
│   ├── stencil_omp.o
│   └── stencil_seq.o
├── LICENSE
├── Makefile
├── module.sh
├── README.md
├── src
│   ├── stencil_hybrid.c
│   ├── stencil_mpi.c
│   ├── stencil_omp.c
│   └── stencil_seq.c
└── tests
```

### 0.2 General Method for Creating an MPI Scenario

#### Step 1: Understand the Cluster Architecture

- **Total Nodes**: Number of physical nodes available.
- **Cores per Node**: Number of cores per node.
- **NUMA Organization**: Number of NUMA nodes per processor/node and associated memory.
- **Threading Capabilities**: Multi-threaded (OpenMP) or single-threaded (pure MPI).

#### Step 2: Define the Allocation Strategy

- **Job Objective**:
  - Maximize performance using all resources.
  - Simulate specific scenarios.
- **Granularity**:
  - **Per Core**: One MPI process per core.
  - **Per NUMA Node**: Group processes by NUMA node for memory optimization.
  - **Per Node**: Use one or more processes per node.

#### Step 3: Build the MPI Command

#### Common Scenarios

| **Scenario**                | **Description**                                           |
| --------------------------- | --------------------------------------------------------- |
| **1 Process per Core**      | Maximizes core usage.                                     |
| **Specific Process Count**  | Allocate a fixed number of processes, distributed evenly. |
| **1 Process per Node**      | Memory-intensive tasks with minimal process usage.        |
| **N Processes per Node**    | Adjust parallelism by controlling processes per node.     |
| **Processes per NUMA Node** | Reduces memory latency for NUMA-sensitive applications.   |

#### Generic Scenario: Flexible Command Structure

```bash
salloc -n <total_process> -N <num_nodes> --exclusive mpirun --map-by ppr:<processes_per_unit>:<unit> ./a.out
```

Replace `<processes_per_unit>`:

- **1**: For 1 process per unit (core, NUMA, or node).
- **N**: For multiple processes per unit.

Replace `<unit>`:

- **core**: One process per core.
- **numa**: Group by NUMA node.
- **node**: Group by entire node.

#### Examples

1. **2 Processes per NUMA Node**:

   ```bash
   salloc -N 4 --exclusive mpirun --map-by ppr:2:numa ./a.out
   ```

2. **1 Process per Core**:

   ```bash
   salloc -N 4 --exclusive mpirun --map-by ppr:1:core ./a.out
   ```

3. **1 Process per Node**:
   ```bash
    salloc -N 4 --exclusive mpirun --map-by ppr:1:node ./a.out
   ```

#### Tips and Tricks

1. Verify Process Allocation
   If you are unsure about how the processes are assigned, you can use the `hostname` command to check:

```bash
salloc -p mistral -N 3 --exclusive mpirun --map-by ppr:4:node hostname
```

2. Check Process Binding
   To validate how processes are bound to resources, use the --report-bindings parameter with mpirun.
   This will provide detailed information about process-to-core or process-to-node bindings.

```bash
mpirun --report-bindings ./a.out
```

---

## Step 1: Understanding the Sequential Version

### 1.1: Code Overview

The initial step is to understand the sequential version of the code.  
This involves reviewing the implementation details and understanding its logic, data flow, and numerical approach.

---

### 1.2: Performance of the Sequential Version

The performance of the sequential version should be analyzed in detail.  
Benchmarking and profiling the code are key to identifying bottlenecks and understanding its execution efficiency.

---

### 1.3: Limitations of the Sequential Version

The sequential version exhibits performance limitations, with observed efficiency far below the nominal processor performance.

While a production-grade code would involve several optimizations, we will not address these here, as the focus is on MPI and OpenMP. However, the following optimizations are typically considered in real-world scenarios:

- **Vectorization**: To fully utilize the processor's vector units.
- **Block-Based Computations**: To optimize cache utilization and reduce memory access latency.
- **Specific Handling of Convergence Tests**: To enable code vectorization and enhance efficiency.
- **3D Calculations**: More realistic and relevant for practical applications. Compared to 2D calculations, 3D simulations have higher arithmetic intensity, which changes performance characteristics.

These optimizations would be indispensable in production-level code, but we will not implement them here because:

- They reduce code readability.
- They are time-consuming to implement.
- They are outside the scope of this project.

Additionally, this code does not aim to replicate physical reality accurately or calibrate the model for precise simulations.

The objective is to focus solely on MPI and OpenMP.
