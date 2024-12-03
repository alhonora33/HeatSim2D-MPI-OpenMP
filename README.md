# HeatSim2D-MPI-OpenMP

This project simulates heat transmission in a 2D solid using numerical methods.  
Dirichlet boundary conditions are applied, and iterations continue until a steady-state is reached (using the 𝜖 convergence criterion).  
OpenMP is employed for shared memory parallelism, while MPI distributes the workload across nodes, ensuring efficient computation for large-scale problems.

## Step 0: Compilation and testing

### 0.1: Compilation

To compile the code, run the following command:

```bash
make
```

### 0.2: Testing

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
