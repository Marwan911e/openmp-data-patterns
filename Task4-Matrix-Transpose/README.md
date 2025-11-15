# Task 4: Matrix Transpose (Block Data Partition)

## Overview

This task implements parallel matrix transpose using **block data partitioning** strategy. The matrix is divided into smaller sub-blocks, and each block transpose operation is assigned to a separate thread. This improves cache locality and memory performance.

## Algorithm

### Matrix Transpose

Transpose swaps rows and columns:

```
Original A:          Transposed B:
[1  2  3]            [1  4  7]
[4  5  6]    →       [2  5  8]
[7  8  9]            [3  6  9]

B[j][i] = A[i][j]
```

### Block Decomposition Strategy

1. **Divide** matrix into blocks of size `block_size × block_size`
2. **Assign** each block to threads dynamically
3. **Transpose** elements within each block
4. **Result** is the complete transposed matrix

## Implementation Details

### Two Parallel Approaches

#### Method 1: Naive Parallel

Direct parallelization of nested loops:

```c
#pragma omp for collapse(2)
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        B[j * N + i] = A[i * N + j];
```

**Problem:** Poor cache performance due to strided memory access.

#### Method 2: Blocked Parallel

Process matrix in blocks for better cache locality:

```c
#pragma omp for collapse(2)
for (int bi = 0; bi < N; bi += block_size)
    for (int bj = 0; bj < N; bj += block_size)
        // Transpose block (bi, bj)
```

**Advantage:** Blocks fit in cache, reducing memory latency.

### Cache Efficiency

**Why blocking helps:**

- Reading A: sequential access (cache-friendly)
- Writing B: strided access (cache-unfriendly)
- Blocking keeps working set small enough to fit in cache

### Key OpenMP Features

- `#pragma omp for collapse(2)` - Parallelize both block dimensions
- `schedule(dynamic)` - Dynamic load balancing
- Thread-safe by design (no race conditions)

## Compilation

```bash
gcc -fopenmp -o matrix_transpose.exe matrix_transpose.c -lm
```

## Usage

```bash
# Default (2048×2048, block_size=64)
./matrix_transpose.exe

# Custom size and block size
./matrix_transpose.exe 4096 128

# Arguments: [matrix_size] [block_size]
```

## Example Output

```
==============================================
    PARALLEL MATRIX TRANSPOSE (BLOCKED)      
==============================================
Matrix Size: 2048 x 2048
Block Size: 64 x 64
Number of threads: 8
==============================================

Initializing matrix...

[1] Running SEQUENTIAL transpose...
    Time: 0.035421 seconds

[2] Running PARALLEL NAIVE transpose...
    Using 8 threads (naive approach)
    Time: 0.012345 seconds

[3] Running PARALLEL BLOCKED transpose...
    Using 8 threads (blocked approach, block=64x64)
    Time: 0.006231 seconds

[4] Verifying results...
    ✓ All transposes correct!

==============================================
  PERFORMANCE SUMMARY
==============================================
Sequential time:      0.035421 seconds
Parallel (naive):     0.012345 seconds (2.87x speedup, 35.9% eff.)
Parallel (blocked):   0.006231 seconds (5.68x speedup, 71.0% eff.)
==============================================

Note: Blocked version typically achieves better
      cache performance, especially for large matrices.
```

## Performance Analysis

### Time Complexity

- **All versions:** O(N²)
- **Speedup depends on:** Memory hierarchy, cache size, thread count

### Expected Performance

| Matrix Size | Sequential | Naive Parallel | Blocked Parallel |
|-------------|-----------|----------------|------------------|
| 512×512     | 0.005s    | 0.002s (2.5x)  | 0.001s (5.0x)   |
| 2048×2048   | 0.035s    | 0.012s (2.9x)  | 0.006s (5.8x)   |
| 4096×4096   | 0.140s    | 0.048s (2.9x)  | 0.022s (6.4x)   |

### Block Size Impact

| Block Size | Cache Fit | Performance |
|------------|-----------|-------------|
| 16×16      | L1 cache  | Good        |
| 64×64      | L2 cache  | Optimal     |
| 256×256    | L3 cache  | Degraded    |
| 1024×1024  | No cache  | Poor        |

**Optimal block size:** Usually 64-128 for modern CPUs.

## Memory Access Patterns

### Sequential Access (Good)

Reading A row-by-row:
```
A[0,0] → A[0,1] → A[0,2] → ... (sequential)
```

### Strided Access (Bad)

Writing B column-by-column:
```
B[0,0] → B[1,0] → B[2,0] → ... (strided by N)
```

### Blocking Benefit

Process small blocks that fit entirely in cache, minimizing cache misses.

## Key Concepts

### 1. Cache Locality

Accessing memory in a cache-friendly pattern improves performance dramatically.

### 2. Spatial Locality

Accessing nearby memory locations (sequential or small strides).

### 3. Temporal Locality

Reusing recently accessed data before it leaves cache.

### 4. Block Decomposition

Dividing computation into cache-sized chunks.

## Testing

### Small Matrix (Visual Verification)

```bash
./matrix_transpose.exe 8 4
```

### Large Matrix (Performance)

```bash
./matrix_transpose.exe 2048 64
./matrix_transpose.exe 4096 128
```

### Block Size Experiment

```bash
./matrix_transpose.exe 2048 16
./matrix_transpose.exe 2048 32
./matrix_transpose.exe 2048 64
./matrix_transpose.exe 2048 128
./matrix_transpose.exe 2048 256
```

## Extensions

### In-Place Transpose

Transpose without additional memory (for square matrices):

```c
for (int i = 0; i < N; i++)
    for (int j = i+1; j < N; j++)
        swap(A[i*N+j], A[j*N+i]);
```

### Non-Square Matrices

Handle M×N matrices:

```c
transpose(A, B, M, N);  // A is M×N, B is N×M
```

### Cache-Oblivious Algorithm

Automatically adapt to any cache size using recursive subdivision.

## Real-World Applications

1. **Linear Algebra:** Matrix operations in BLAS libraries
2. **Image Processing:** Rotate images 90 degrees
3. **Scientific Computing:** Array layout transformations
4. **Data Analysis:** Pivot tables, data reshaping
5. **Deep Learning:** Tensor transpositions

## Common Issues

1. **Poor speedup?**
   - Increase matrix size
   - Adjust block size
   - Check cache size

2. **Blocked slower than naive?**
   - Block size too small (overhead)
   - Block size too large (doesn't fit cache)
   - Try block_size = 64 or 128

3. **Incorrect results?**
   - Check block boundary calculations
   - Verify index arithmetic

## Performance Tips

1. **Choose optimal block size** based on cache line size (typically 64 bytes)
2. **Use collapse(2)** to parallelize both loop dimensions
3. **Dynamic scheduling** handles irregular block boundaries
4. **Align data** to cache line boundaries for best performance

## Advanced Optimizations

### SIMD Vectorization

Use compiler intrinsics for vectorized operations:

```c
// SSE/AVX instructions for 4-8 doubles at once
```

### Prefetching

Explicitly prefetch next block:

```c
_mm_prefetch(&A[(i+block_size)*N], _MM_HINT_T0);
```

### Non-Temporal Stores

For large matrices that don't fit in cache:

```c
_mm_stream_pd(&B[j*N+i], value);
```

## References

- Cache-Oblivious Algorithms: Frigo et al.
- OpenMP Collapse: https://www.openmp.org/spec-html/5.0/openmpsu42.html
- Matrix Transpose Optimization: https://csapp.cs.cmu.edu/3e/waside/waside-blocking.pdf
- Memory Hierarchy: Computer Architecture by Hennessy & Patterson

