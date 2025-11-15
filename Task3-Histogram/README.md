# Task 3: Parallel Histogram Computation

## Overview

This task implements parallel histogram computation using **data partitioning** strategy. Given a large array of integers (0-9), the data is decomposed among threads to build a shared histogram with proper synchronization.

## Algorithm

### Histogram Computation

A histogram counts the frequency of each distinct value in a dataset:

```
Input:  [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, ...]
Output: {0: 102, 1: 985, 2: 1023, ..., 9: 987}
```

### Two Parallel Approaches

#### Method 1: Atomic Operations

- Each thread directly updates the shared histogram
- Uses atomic operations for thread-safe increments
- Simple but may have contention on histogram bins

#### Method 2: Local Histograms + Reduction

- Each thread maintains a private local histogram
- After computation, combine local histograms into global
- Less contention, typically faster

## Implementation Details

### Key OpenMP Features

**Atomic Approach:**
```c
#pragma omp atomic
histogram[data[i]]++;
```

**Reduction Approach:**
```c
// Private local histogram per thread
int local_hist[NUM_BINS] = {0};

#pragma omp for nowait
for (int i = 0; i < size; i++)
    local_hist[data[i]]++;

#pragma omp critical
for (int bin = 0; bin < NUM_BINS; bin++)
    histogram[bin] += local_hist[bin];
```

### Data Decomposition

```
Array (N elements) → Partitions (N/P elements per thread)

Each thread processes its assigned portion:
Thread 0: elements [0, N/P)
Thread 1: elements [N/P, 2N/P)
...
```

## Compilation

```bash
gcc -fopenmp -o histogram.exe histogram.c
```

## Usage

```bash
# Default size (10 million elements)
./histogram.exe

# Custom size
./histogram.exe 100000000

# Arguments: [array_size]
```

## Example Output

```
==============================================
    PARALLEL HISTOGRAM COMPUTATION (0-9)     
==============================================
Array size: 10000000 elements
Number of bins: 10 (0-9)
Number of threads: 8
==============================================

Generating random data (0-9)...

[1] Running SEQUENTIAL histogram...
    Time: 0.024531 seconds

    Sequential Histogram:
    --------------------------------
    0:    1000234 |██████████████████████████████████████████████████
    1:     999821 |█████████████████████████████████████████████████
    2:    1000145 |██████████████████████████████████████████████████
    3:     999678 |█████████████████████████████████████████████████
    4:    1000512 |██████████████████████████████████████████████████
    5:     999234 |█████████████████████████████████████████████████
    6:    1000876 |██████████████████████████████████████████████████
    7:     999543 |█████████████████████████████████████████████████
    8:    1000123 |██████████████████████████████████████████████████
    9:     999834 |█████████████████████████████████████████████████
    --------------------------------
    Total: 10000000 elements

[2] Running PARALLEL histogram (ATOMIC)...
    Using 8 threads with atomic operations
    Time: 0.012345 seconds

[3] Running PARALLEL histogram (REDUCTION)...
    Using 8 threads with local histogram reduction
    Time: 0.004231 seconds

[4] Verifying results...
    ✓ All histograms match! Correctness verified.

==============================================
  PERFORMANCE SUMMARY
==============================================
Sequential time:      0.024531 seconds
Parallel (atomic):    0.012345 seconds (1.99x speedup, 24.9% efficiency)
Parallel (reduction): 0.004231 seconds (5.80x speedup, 72.5% efficiency)
==============================================

Note: Reduction method is typically faster due to
      reduced synchronization overhead.
```

## Performance Analysis

### Time Complexity

- **Sequential:** O(N)
- **Parallel:** O(N/P) + O(B) where B = number of bins

### Expected Speedup

| Array Size | Threads | Atomic Speedup | Reduction Speedup |
|------------|---------|----------------|-------------------|
| 1M         | 4       | ~2.0x          | ~3.5x            |
| 10M        | 8       | ~2.5x          | ~6.0x            |
| 100M       | 16      | ~3.0x          | ~12x             |

### Why Reduction is Faster

**Atomic Method:**
- High contention on shared bins
- Cache coherence overhead
- Memory synchronization penalty

**Reduction Method:**
- No contention during counting phase
- Cache-friendly local arrays
- Single reduction phase at the end

## Synchronization Strategies

### 1. Atomic Operations

**Pros:**
- Simple to implement
- Low memory overhead

**Cons:**
- Contention bottleneck
- Cache line bouncing
- Slower for high thread counts

### 2. Local Histogram Reduction

**Pros:**
- Minimal contention
- Cache-efficient
- Scales better with threads

**Cons:**
- Higher memory usage (P × B bins)
- Reduction overhead

### 3. Array Reduction (Alternative)

OpenMP 4.5+ supports array reduction:

```c
int histogram[NUM_BINS] = {0};

#pragma omp parallel for reduction(+:histogram[:NUM_BINS])
for (int i = 0; i < size; i++) {
    histogram[data[i]]++;
}
```

## Key Concepts

### 1. Data Partitioning

Dividing the input array among threads for parallel processing.

### 2. Race Conditions

Without synchronization, multiple threads updating the same bin causes incorrect results.

### 3. Contention

When many threads try to update the same memory location simultaneously.

### 4. Reduction Pattern

Combining partial results from threads into a final result.

## Testing

### Small Test
```bash
./histogram.exe 1000
```

### Large Test
```bash
./histogram.exe 100000000
```

### Verification

The program automatically verifies that all three methods produce identical results.

## Extensions

### Multi-Range Histogram

Support different bin ranges:

```c
// 0-255 for byte histograms (image processing)
#define NUM_BINS 256
```

### Weighted Histogram

Count with weights instead of simple counts:

```c
histogram[data[i]] += weights[i];
```

### 2D Histogram

For two variables simultaneously:

```c
histogram_2d[data_x[i]][data_y[i]]++;
```

## Real-World Applications

1. **Image Processing:** Color distribution analysis
2. **Data Analytics:** Frequency distribution
3. **Statistics:** Probability distribution estimation
4. **Machine Learning:** Feature engineering

## Common Issues

1. **Race conditions?**
   - Ensure atomic or proper synchronization
   - Use reduction pattern

2. **Poor speedup with atomic?**
   - Expected for high contention
   - Switch to reduction method

3. **Memory errors?**
   - Check bin indices (must be 0-9)
   - Verify array bounds

## Performance Tips

1. **Use reduction for best performance**
2. **Increase array size** for better parallelization
3. **Adjust thread count** to match CPU cores
4. **Consider cache effects** for large bin counts

## References

- OpenMP Atomic: https://www.openmp.org/spec-html/5.0/openmpsu95.html
- Reduction Pattern: https://patterns.eecs.berkeley.edu/
- Histogram Algorithms: https://en.wikipedia.org/wiki/Histogram
- Parallel Computing Patterns: Mattson, Sanders, Massingill

