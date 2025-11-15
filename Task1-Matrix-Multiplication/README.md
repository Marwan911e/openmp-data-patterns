# Task 1: Large Matrix Multiplication (Block Decomposition)

## Overview

This task implements parallel matrix multiplication using **block decomposition** strategy. The matrices are divided into smaller sub-blocks, and each thread multiplies block pairs to improve cache locality and achieve parallelism.

## Algorithm

### Block Decomposition Strategy

1. **Divide** matrices into blocks of size `block_size × block_size`
2. **Assign** each output block to threads dynamically
3. **Multiply** corresponding block pairs (A_block × B_block)
4. **Accumulate** partial results using atomic operations

### Matrix Multiplication Formula

```
C[i][j] = Σ(k=0 to N-1) A[i][k] × B[k][j]
```

In blocked form:
```
C_block[bi][bj] = Σ(bk) A_block[bi][bk] × B_block[bk][bj]
```

## Implementation Details

### Key OpenMP Features

- `#pragma omp parallel for collapse(2)` - Parallelizes nested loops over blocks
- `#pragma omp atomic` - Thread-safe accumulation of results
- `schedule(dynamic)` - Dynamic load balancing for blocks

### Data Decomposition

```
Matrix (N×N) → Blocks (block_size × block_size)

Example: 512×512 matrix with 64×64 blocks = 8×8 = 64 blocks
```

### Synchronization

Uses **atomic operations** for result accumulation to avoid race conditions:

```c
#pragma omp atomic
C[i * N + j] += sum;
```

## Compilation

```bash
gcc -fopenmp -o matrix_multiplication.exe matrix_multiplication.c -lm
```

## Usage

```bash
# Default size (512×512, block_size=64)
./matrix_multiplication.exe

# Custom size
./matrix_multiplication.exe 1024 128

# Arguments: [matrix_size] [block_size]
```

## Example Output

```
==============================================
  PARALLEL MATRIX MULTIPLICATION (BLOCKED)   
==============================================
Matrix Size: 512 x 512
Block Size: 64 x 64
Number of threads: 8
==============================================

Initializing matrices...

[1] Running SEQUENTIAL multiplication...
    Time: 0.285432 seconds

[2] Running PARALLEL BLOCKED multiplication...
    Using 8 threads for blocked multiplication
    Time: 0.048621 seconds

[3] Verifying results...
    ✓ Results match! Correctness verified.

==============================================
  PERFORMANCE SUMMARY
==============================================
Sequential time:   0.285432 seconds
Parallel time:     0.048621 seconds
Speedup:           5.87x
Efficiency:        73.4%
==============================================
```

## Performance Analysis

### Time Complexity

- **Sequential:** O(N³)
- **Parallel:** O(N³ / P) where P = number of threads

### Expected Speedup

| Matrix Size | Threads | Speedup |
|-------------|---------|---------|
| 256×256     | 4       | ~3.2x   |
| 512×512     | 8       | ~5.8x   |
| 1024×1024   | 8       | ~6.5x   |
| 2048×2048   | 16      | ~12x    |

### Block Size Impact

- **Small blocks** (16×16): More parallelism, worse cache locality
- **Large blocks** (128×128): Better cache use, less parallelism
- **Optimal** (64×64 - 128×128): Balance between the two

## Advantages of Block Decomposition

1. **Cache Efficiency:** Blocks fit in L1/L2 cache
2. **Load Balancing:** Dynamic scheduling handles uneven loads
3. **Scalability:** Works well with many threads
4. **Memory Access:** Improved spatial locality

## Key Concepts

### 1. Data Decomposition

Dividing the problem domain (matrices) into smaller chunks that can be processed independently.

### 2. Cache Locality

Processing data in blocks that fit in cache reduces memory latency.

### 3. Synchronization

Using atomic operations to safely update shared output matrix.

### 4. Dynamic Scheduling

Distributing block computations dynamically to balance thread workload.

## Testing

Run with small matrices to verify correctness:

```bash
./matrix_multiplication.exe 8 4
```

Then scale up to measure performance:

```bash
./matrix_multiplication.exe 512 64
./matrix_multiplication.exe 1024 128
./matrix_multiplication.exe 2048 256
```

## Common Issues

1. **Poor speedup?**
   - Increase matrix size (overhead dominates for small matrices)
   - Adjust block size
   - Check CPU core availability

2. **Incorrect results?**
   - Race condition in accumulation (ensure atomic is used)
   - Block boundary calculations

3. **Memory issues?**
   - Large matrices may exceed RAM
   - Reduce size or use out-of-core algorithms

## Further Optimization

- **Cache-oblivious algorithms** for automatic blocking
- **SIMD instructions** for vectorization
- **GPU acceleration** with CUDA/OpenCL
- **Strassen's algorithm** to reduce complexity

## References

- OpenMP parallel for: https://www.openmp.org/spec-html/5.0/openmpsu39.html
- Matrix multiplication optimization: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm
- Cache-aware algorithms: Patterns for Parallel Programming (Mattson et al.)

