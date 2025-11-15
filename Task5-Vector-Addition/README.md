# Task 5: Vector Addition (Element Partitioning)

## Overview

This task implements parallel vector addition using **element-wise data partitioning**. Two large vectors are added in parallel by distributing elements evenly among threads using OpenMP.

## Algorithm

### Vector Addition

Element-wise addition of two vectors:

```
A = [a₀, a₁, a₂, ..., aₙ₋₁]
B = [b₀, b₁, b₂, ..., bₙ₋₁]
C = [c₀, c₁, c₂, ..., cₙ₋₁]

where cᵢ = aᵢ + bᵢ for all i
```

### Element Partitioning

Divide vector elements among threads:

```
Thread 0: C[0...n/P]     = A[0...n/P]     + B[0...n/P]
Thread 1: C[n/P...2n/P]  = A[n/P...2n/P]  + B[n/P...2n/P]
...
Thread P: C[...n]        = A[...n]        + B[...n]
```

## Implementation Details

### Two Scheduling Strategies

#### Static Scheduling (Default)

Each thread gets a contiguous chunk of elements:

```c
#pragma omp for schedule(static)
for (int i = 0; i < size; i++)
    C[i] = A[i] + B[i];
```

- **Predictable:** Each thread knows its range upfront
- **Cache-friendly:** Contiguous memory access
- **Best for:** Uniform workloads (like vector addition)

#### Dynamic Scheduling

Work is distributed dynamically in smaller chunks:

```c
#pragma omp for schedule(dynamic, chunk_size)
for (int i = 0; i < size; i++)
    C[i] = A[i] + B[i];
```

- **Flexible:** Runtime work distribution
- **Overhead:** Thread synchronization for each chunk
- **Best for:** Irregular workloads (not vector addition)

### Key OpenMP Features

- `#pragma omp parallel for` - Parallel loop
- `schedule(static)` - Static work distribution
- No synchronization needed - independent operations

## Compilation

```bash
gcc -fopenmp -o vector_addition.exe vector_addition.c
```

## Usage

```bash
# Default size (100 million elements)
./vector_addition.exe

# Custom size
./vector_addition.exe 50000000

# Arguments: [vector_size]
```

## Example Output

```
==============================================
     PARALLEL VECTOR ADDITION (ELEMENT)      
==============================================
Vector size: 100000000 elements
Memory: 762.94 MB per vector
Number of threads: 8
==============================================

Initializing vectors...

[1] Running SEQUENTIAL vector addition...
    Time: 0.245312 seconds
    Throughput: 407.63 Million ops/sec

[2] Running PARALLEL vector addition (STATIC)...
    Using 8 threads with STATIC scheduling
    Chunk size per thread: ~12500000 elements
    Time: 0.034521 seconds
    Throughput: 2897.34 Million ops/sec

[3] Running PARALLEL vector addition (DYNAMIC)...
    Using 8 threads with DYNAMIC scheduling
    Time: 0.035234 seconds
    Throughput: 2838.12 Million ops/sec

[4] Verifying results...
    ✓ All results match! Correctness verified.

==============================================
  PERFORMANCE SUMMARY
==============================================
Sequential time:     0.245312 seconds
Parallel (static):   0.034521 seconds (7.11x speedup, 88.8% eff.)
Parallel (dynamic):  0.035234 seconds (6.96x speedup, 87.0% eff.)
==============================================

Note: Vector addition is memory-bound.
      Speedup limited by memory bandwidth.
```

## Performance Analysis

### Time Complexity

- **Sequential:** O(N)
- **Parallel:** O(N/P) where P = number of threads

### Memory Bandwidth

Vector addition is **memory-bound**, not compute-bound:

- **Arithmetic intensity:** Very low (1 FLOP per 3 memory accesses)
- **Bottleneck:** Memory bandwidth, not CPU
- **Speedup limit:** System memory bandwidth

### Expected Performance

| Vector Size | Threads | Speedup | Efficiency |
|-------------|---------|---------|------------|
| 10M         | 4       | 3.5x    | 87%        |
| 100M        | 8       | 7.1x    | 89%        |
| 1B          | 16      | 12x     | 75%        |

### Roofline Analysis

```
Theoretical speedup ≤ min(P, Bandwidth_parallel / Bandwidth_sequential)
```

For modern systems with multi-channel memory:
- Sequential: ~10 GB/s (single channel)
- Parallel: ~40-80 GB/s (quad channel)
- Max speedup: 4-8x

## Advantages of Element Partitioning

1. **Simplicity:** Trivial to implement
2. **Perfect Load Balance:** Equal work per thread
3. **No Synchronization:** Independent operations
4. **Cache-Friendly:** Sequential memory access
5. **Scalability:** Near-linear speedup (memory-bound)

## Key Concepts

### 1. Data Parallelism

Each thread performs the same operation on different data.

### 2. Element-Wise Operations

Operations that are independent for each element.

### 3. Memory-Bound Computation

Performance limited by memory bandwidth, not CPU speed.

### 4. NUMA Effects

On multi-socket systems, memory locality matters:

```c
// First-touch policy: allocate where used
#pragma omp parallel for
for (int i = 0; i < size; i++)
    A[i] = 0.0;  // Thread initializes its portion
```

## Testing

### Small Vector (Visual Verification)

```bash
./vector_addition.exe 20
```

### Large Vector (Performance)

```bash
./vector_addition.exe 100000000
./vector_addition.exe 1000000000
```

### Scaling Study

```bash
export OMP_NUM_THREADS=1; ./vector_addition.exe 100000000
export OMP_NUM_THREADS=2; ./vector_addition.exe 100000000
export OMP_NUM_THREADS=4; ./vector_addition.exe 100000000
export OMP_NUM_THREADS=8; ./vector_addition.exe 100000000
```

## Extensions

### Fused Operations

Combine multiple operations:

```c
// C = alpha*A + beta*B (AXPY)
C[i] = alpha * A[i] + beta * B[i];
```

### SIMD Vectorization

Use compiler auto-vectorization or explicit SIMD:

```c
#pragma omp simd
for (int i = 0; i < size; i++)
    C[i] = A[i] + B[i];
```

### GPU Acceleration

For extreme performance, use GPU:

```c
// CUDA kernel
__global__ void vector_add(float *A, float *B, float *C, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) C[i] = A[i] + B[i];
}
```

## Real-World Applications

1. **Scientific Computing:** BLAS Level 1 operations
2. **Machine Learning:** Gradient updates, tensor operations
3. **Signal Processing:** Audio/video processing
4. **Graphics:** Vertex transformations
5. **Finance:** Portfolio calculations

## Common Issues

1. **Low speedup?**
   - Memory bandwidth saturated
   - Use larger vectors
   - Check NUMA settings

2. **Dynamic slower than static?**
   - Expected for uniform workloads
   - Dynamic has scheduling overhead
   - Use static for vector operations

3. **Poor scaling beyond 8 threads?**
   - Memory bandwidth limit reached
   - Not a code issue, hardware limitation

## Performance Tips

1. **Use static scheduling** for uniform workloads
2. **Large vectors** for better parallelization
3. **NUMA-aware allocation** on multi-socket systems
4. **Combine operations** to improve arithmetic intensity
5. **SIMD directives** for additional speedup

## Scheduling Comparison

### Static Scheduling

**Pros:**
- Minimal overhead
- Predictable distribution
- Cache-friendly
- Best for uniform work

**Cons:**
- Poor for irregular workloads

### Dynamic Scheduling

**Pros:**
- Handles irregular workloads
- Automatic load balancing

**Cons:**
- Synchronization overhead
- Less cache-friendly
- Overkill for vector addition

## Advanced Optimization

### Cache Blocking

Process vectors in cache-sized blocks:

```c
for (int block = 0; block < N; block += BLOCK_SIZE)
    #pragma omp parallel for
    for (int i = block; i < block + BLOCK_SIZE; i++)
        C[i] = A[i] + B[i];
```

### Software Pipelining

Overlap computation and memory access:

```c
_mm_prefetch(&A[i + 64], _MM_HINT_T0);
```

## References

- OpenMP Scheduling: https://www.openmp.org/spec-html/5.0/openmpsu42.html
- BLAS Level 1: https://www.netlib.org/blas/#_level_1
- Memory Bandwidth: https://www.cs.virginia.edu/stream/
- Roofline Model: Williams et al., "Roofline: An Insightful Visual Performance Model"
- NUMA: https://www.kernel.org/doc/html/latest/vm/numa.html

