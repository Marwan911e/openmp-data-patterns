# Task 6: Parallel Sparse Matrix-Vector Multiplication

## Overview

This task implements parallel sparse matrix-vector multiplication (SpMV) using **CSR (Compressed Sparse Row)** format. The matrix is represented efficiently, and row blocks are assigned to threads with dynamic scheduling to handle irregular workloads.

## Algorithm

### Sparse Matrix-Vector Multiplication

Compute y = A × x where A is sparse:

```
y[i] = Σⱼ A[i,j] × x[j]  (only for non-zero A[i,j])
```

### CSR (Compressed Sparse Row) Format

Efficient representation for sparse matrices:

```
Dense Matrix:          CSR Representation:
[2.0  0   3.0]        values:      [2.0, 3.0, 4.0, 5.0, 6.0]
[0    0   4.0]        col_indices: [0,   2,   2,   0,   1  ]
[5.0  6.0 0  ]        row_ptr:     [0,   2,   3,   5      ]
```

**Three arrays:**
1. `values[]` - Non-zero values (size = nnz)
2. `col_indices[]` - Column index for each value (size = nnz)
3. `row_ptr[]` - Start index of each row (size = num_rows + 1)

## Implementation Details

### CSR Format Advantages

1. **Memory efficient:** Only stores non-zeros
2. **Fast row access:** Direct indexing via row_ptr
3. **Cache-friendly:** Sequential access within rows
4. **Standard format:** Used in scientific libraries (BLAS, cuSPARSE)

### Memory Savings

For an N×N matrix with density d:
- **Dense storage:** N² × 8 bytes (double)
- **CSR storage:** (nnz × 12 + N × 4) bytes ≈ d×N² × 12 bytes

Example (10,000×10,000, 5% density):
- Dense: 763 MB
- CSR: 4.6 MB (165× smaller!)

### Scheduling Strategies

#### Static Scheduling

Each thread gets contiguous rows:

```c
#pragma omp for schedule(static)
for (int i = 0; i < num_rows; i++)
    // Compute y[i]
```

**Problem:** Irregular row lengths cause load imbalance.

#### Dynamic Scheduling

Rows distributed dynamically:

```c
#pragma omp for schedule(dynamic, chunk_size)
for (int i = 0; i < num_rows; i++)
    // Compute y[i]
```

**Advantage:** Better load balancing for irregular sparsity patterns.

### Key OpenMP Features

- `#pragma omp for schedule(dynamic)` - Dynamic work distribution
- Small chunk size (10-100 rows) for fine-grained load balancing
- No race conditions (each thread writes to different y[i])

## Compilation

```bash
gcc -fopenmp -o sparse_matrix_vector.exe sparse_matrix_vector.c -lm
```

## Usage

```bash
# Default (10,000 rows, 5% density)
./sparse_matrix_vector.exe

# Custom size and density
./sparse_matrix_vector.exe 50000 0.01

# Arguments: [num_rows] [density]
```

## Example Output

```
==============================================
  SPARSE MATRIX-VECTOR MULTIPLICATION (CSR)  
==============================================
Matrix size: 10000 x 10000
Density: 5.00%
Number of threads: 8
==============================================

Generating random sparse matrix...
Matrix created: 4998752 non-zero elements out of 100000000 (5.00%)
Memory saved: 57.22 MB (vs 762.94 MB for dense)

[1] Running SEQUENTIAL SpMV...
    Time: 0.008234 seconds
    GFLOPS: 1.215

[2] Running PARALLEL SpMV (STATIC)...
    Using 8 threads with STATIC scheduling
    Time: 0.002145 seconds
    GFLOPS: 4.658

[3] Running PARALLEL SpMV (DYNAMIC)...
    Using 8 threads with DYNAMIC scheduling
    Time: 0.001432 seconds
    GFLOPS: 6.982

[4] Verifying results...
    ✓ All results match! Correctness verified.

==============================================
  PERFORMANCE SUMMARY
==============================================
Sequential time:     0.008234 seconds
Parallel (static):   0.002145 seconds (3.84x speedup, 48.0% eff.)
Parallel (dynamic):  0.001432 seconds (5.75x speedup, 71.9% eff.)
==============================================

Note: Dynamic scheduling typically performs better
      for irregular sparse patterns.
```

## Performance Analysis

### Time Complexity

- **Sequential:** O(nnz) where nnz = number of non-zeros
- **Parallel:** O(nnz / P) with load balancing

### Expected Speedup

| Matrix Size | Density | Threads | Static | Dynamic |
|-------------|---------|---------|--------|---------|
| 10K×10K     | 5%      | 4       | 3.0x   | 3.8x    |
| 10K×10K     | 5%      | 8       | 4.2x   | 6.5x    |
| 100K×100K   | 1%      | 16      | 6.0x   | 12x     |

### Why Dynamic Scheduling Wins

**Irregular Workload:**
- Some rows have many non-zeros (heavy)
- Some rows have few non-zeros (light)
- Static gives each thread equal rows → unequal work
- Dynamic distributes work evenly at runtime

### Sparsity Patterns

Different patterns affect performance:

1. **Random sparse** (uniform): Static OK, Dynamic better
2. **Diagonal** (structured): Static good
3. **Power-law** (real graphs): Dynamic much better
4. **Block-structured**: Depends on block distribution

## Key Concepts

### 1. Sparse Matrix Formats

- **CSR (this task):** Row-major, fast row access
- **CSC:** Column-major, fast column access
- **COO:** Coordinate format, flexible
- **ELL:** ELLPACK, GPU-friendly

### 2. Load Balancing

Distributing work evenly despite irregular workloads.

### 3. Irregular Parallelism

When work per iteration varies significantly.

### 4. Memory Bandwidth

SpMV is typically memory-bound, not compute-bound.

## Testing

### Small Matrix (Visual)

```bash
./sparse_matrix_vector.exe 10 0.3
```

### Large Matrix (Performance)

```bash
./sparse_matrix_vector.exe 50000 0.05
./sparse_matrix_vector.exe 100000 0.01
```

### Density Experiments

```bash
./sparse_matrix_vector.exe 10000 0.01
./sparse_matrix_vector.exe 10000 0.05
./sparse_matrix_vector.exe 10000 0.10
./sparse_matrix_vector.exe 10000 0.20
```

## Extensions

### Other Sparse Formats

**COO (Coordinate) Format:**

```c
struct COO {
    int *row_indices;
    int *col_indices;
    double *values;
    int nnz;
};
```

**CSC (Compressed Sparse Column):**

Like CSR but column-major ordering.

### Sparse Matrix-Matrix Multiplication

Extend to y = A × B where both are sparse.

### Iterative Solvers

SpMV is the kernel of:
- Conjugate Gradient (CG)
- GMRES
- Jacobi iteration

### GPU Acceleration

CUDA implementation with CSR format:

```c
__global__ void spmv_csr_kernel(...)
```

## Real-World Applications

1. **Scientific Computing:** Finite element analysis, CFD
2. **Graph Analytics:** PageRank, graph algorithms
3. **Machine Learning:** Sparse neural networks
4. **Recommender Systems:** Collaborative filtering
5. **Network Analysis:** Social networks, web graphs

## Common Issues

1. **Static slower than expected?**
   - Irregular sparsity pattern
   - Switch to dynamic scheduling

2. **Poor speedup overall?**
   - Memory-bound operation
   - Limited by bandwidth
   - Try larger matrices

3. **Memory errors?**
   - Check CSR format construction
   - Verify row_ptr boundaries

## Performance Tips

1. **Use dynamic scheduling** for irregular patterns
2. **Tune chunk size** (typically 10-100 rows)
3. **NUMA-aware allocation** for large matrices
4. **Consider format conversion** (e.g., CSR to ELL for GPU)
5. **Block CSR** for better cache utilization

## Advanced Optimizations

### Register Blocking

Process multiple rows per thread to reduce memory traffic.

### Software Pipelining

Overlap computation with memory fetches:

```c
_mm_prefetch(&values[row_ptr[i+1]], _MM_HINT_T0);
```

### Mixed Precision

Use float for storage, double for computation.

### GPU Offloading

For very large matrices:

```c
#pragma omp target teams distribute parallel for
for (int i = 0; i < num_rows; i++)
    // SpMV kernel
```

## Comparison with Dense

| Aspect       | Dense MatVec | Sparse MatVec (CSR) |
|--------------|-------------|---------------------|
| Memory       | O(N²)       | O(nnz)              |
| Time         | O(N²)       | O(nnz)              |
| Cache        | Good        | Irregular           |
| Parallelism  | Easy        | Load balancing      |

## References

- CSR Format: https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_(CSR,_CRS_or_Yale_format)
- SpMV Optimization: Williams et al., "Optimization of Sparse Matrix-Vector Multiplication on Emerging Multicore Platforms"
- Dynamic Scheduling: https://www.openmp.org/spec-html/5.0/openmpsu42.html
- Scientific Libraries: Intel MKL, cuSPARSE, Eigen
- Graph Applications: NetworkX, igraph

