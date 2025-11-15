# Quick Reference Guide - OpenMP Performance

## 🚫 Common Anti-Patterns (What NOT to Do)

### ❌ Anti-Pattern 1: Atomic in Hot Loop
```c
// BAD: Atomic on every iteration
#pragma omp parallel for
for (int i = 0; i < N; i++) {
    #pragma omp atomic
    shared_variable += data[i];  // ❌ N atomic operations!
}
```

### ✅ Correct Pattern: Local Reduction
```c
// GOOD: Use local variables, combine at end
#pragma omp parallel
{
    int local_sum = 0;
    #pragma omp for nowait
    for (int i = 0; i < N; i++) {
        local_sum += data[i];  // ✅ No synchronization
    }
    #pragma omp atomic
    shared_variable += local_sum;  // ✅ Only 1 atomic per thread
}
```

---

### ❌ Anti-Pattern 2: Over-Parallelization
```c
// BAD: Parallelizing accumulation dimension
#pragma omp for collapse(3)
for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        for (int k = 0; k < N; k++) {  // ❌ Multiple threads write to C[i][j]
            #pragma omp atomic
            C[i][j] += A[i][k] * B[k][j];
        }
    }
}
```

### ✅ Correct Pattern: Parallelize Output Only
```c
// GOOD: Only parallelize independent dimensions
#pragma omp for collapse(2)
for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
        double sum = 0.0;
        for (int k = 0; k < N; k++) {  // ✅ Sequential, no conflicts
            sum += A[i][k] * B[k][j];
        }
        C[i][j] = sum;  // ✅ Each thread owns C[i][j]
    }
}
```

---

### ❌ Anti-Pattern 3: False Sharing
```c
// BAD: Adjacent threads write to nearby memory
int results[8];  // ❌ On same cache line
#pragma omp parallel
{
    int tid = omp_get_thread_num();
    results[tid] = compute();  // ❌ Cache line bouncing
}
```

### ✅ Correct Pattern: Proper Alignment
```c
// GOOD: Pad to avoid false sharing
int results[8][16];  // ✅ Each on different cache line
#pragma omp parallel
{
    int tid = omp_get_thread_num();
    results[tid][0] = compute();  // ✅ No false sharing
}
```

---

## ✅ Best Practices Checklist

### Before Parallelizing
- [ ] Identify independent work units
- [ ] Check for data dependencies
- [ ] Verify no shared writes in hot loops
- [ ] Choose appropriate decomposition strategy

### During Implementation
- [ ] Minimize synchronization points
- [ ] Use local variables in threads
- [ ] Combine results at the end only
- [ ] Choose correct scheduling policy

### After Implementation
- [ ] Test correctness vs sequential version
- [ ] Measure actual speedup
- [ ] Profile for bottlenecks
- [ ] Document performance characteristics

---

## 📊 Scheduling Guide

### Static Scheduling
```c
#pragma omp for schedule(static)
```
**Use when:**
- Uniform workload (equal work per iteration)
- Memory-bound operations
- Want minimal overhead

**Examples:** Vector addition, dense matrix operations

### Dynamic Scheduling
```c
#pragma omp for schedule(dynamic, chunk_size)
```
**Use when:**
- Irregular workload (varying work per iteration)
- Unknown work distribution
- Load balancing important

**Examples:** Sparse matrices, tree traversal, irregular loops

### Guided Scheduling
```c
#pragma omp for schedule(guided)
```
**Use when:**
- Decreasing work per iteration
- Want dynamic benefits with less overhead

---

## 🎯 Data Decomposition Strategies

### 1. Block Decomposition
**Best for:** Matrices, 2D data
```c
#pragma omp for collapse(2)
for (int bi = 0; bi < N; bi += BLOCK)
    for (int bj = 0; bj < N; bj += BLOCK)
        process_block(bi, bj);
```

### 2. Element Partitioning
**Best for:** Vectors, 1D arrays
```c
#pragma omp for
for (int i = 0; i < N; i++)
    C[i] = A[i] + B[i];
```

### 3. Row/Column Decomposition
**Best for:** Sparse matrices, irregular data
```c
#pragma omp for schedule(dynamic)
for (int row = 0; row < num_rows; row++)
    process_row(row);
```

---

## 🔍 Performance Debugging

### Why is my parallel code slower?

#### Problem: Negative Speedup
**Symptoms:** Parallel version slower than sequential

**Common Causes:**
1. ❌ Atomic/critical in hot loop → Remove or use reduction
2. ❌ Input too small → Use larger problem size
3. ❌ Memory-bound operation → Limited by bandwidth, not CPU
4. ❌ Too many threads → Reduce to match CPU cores

#### Problem: No Speedup
**Symptoms:** Parallel same speed as sequential

**Common Causes:**
1. ❌ Serialization bottleneck → Check for implicit barriers
2. ❌ Load imbalance → Try dynamic scheduling
3. ❌ False sharing → Pad data structures
4. ❌ Memory bandwidth saturated → Expected for memory-bound ops

#### Problem: Incorrect Results
**Symptoms:** Parallel gives different answer than sequential

**Common Causes:**
1. ❌ Race condition → Add synchronization or restructure
2. ❌ Uninitialized shared data → Use `shared` and `private` correctly
3. ❌ Missing reduction clause → Use `reduction(+:var)`

---

## 💡 Quick Fixes

### Fix 1: Reduction Pattern
```c
// Before: Atomic in loop
for (i) { #pragma omp atomic; sum += x[i]; }

// After: Use OpenMP reduction
#pragma omp parallel for reduction(+:sum)
for (i) { sum += x[i]; }
```

### Fix 2: Local Accumulation
```c
// Before: Shared counter
for (i) { #pragma omp atomic; histogram[data[i]]++; }

// After: Local histograms
int local_hist[BINS] = {0};
#pragma omp for
for (i) { local_hist[data[i]]++; }
#pragma omp critical
{ for (b) histogram[b] += local_hist[b]; }
```

### Fix 3: Proper Work Partitioning
```c
// Before: Parallelize all dimensions
#pragma omp for collapse(3)
for (i) for (j) for (k) { atomic update C[i][j]; }

// After: Parallelize output only
#pragma omp for collapse(2)
for (i) for (j) { sum=0; for (k) sum+=...; C[i][j]=sum; }
```

---

## 📏 Performance Expectations

### Compute-Bound Operations
- **Examples:** Matrix multiplication, FFT
- **Expected:** Near-linear speedup (e.g., 8x on 8 cores)
- **Limit:** Number of CPU cores

### Memory-Bound Operations
- **Examples:** Vector addition, transpose, sparse matrix
- **Expected:** Modest speedup (2-8x)
- **Limit:** Memory bandwidth (typically 4-8 channels)

### I/O-Bound Operations
- **Examples:** File encryption, data loading
- **Expected:** Limited speedup (2-4x)
- **Limit:** Disk/network bandwidth

---

## 🎓 Key Principles

### 1. No Shared Writes in Hot Loops
> "If multiple threads need to update the same variable in a hot loop, you're doing it wrong."

### 2. Synchronization is Expensive
> "Every atomic/critical/barrier is a potential bottleneck. Minimize them."

### 3. Match Parallelism to Problem Size
> "Parallel overhead is fixed. Make the problem large enough to amortize it."

### 4. Memory Bandwidth is Real
> "You can't parallelize your way past memory bandwidth limits."

### 5. Profile Before Optimizing
> "Measure, don't guess. Profile to find real bottlenecks."

---

## 📚 Task-Specific Tips

### Task 1: Matrix Multiplication
- ✅ Parallelize (i,j), sequential k
- ✅ Use blocking for cache efficiency
- ❌ Don't parallelize k dimension

### Task 3: Histogram
- ✅ Use local histograms + reduction
- ✅ Critical section for final merge
- ❌ Don't use atomic on every element

### Task 4: Transpose
- ✅ Blocked for cache locality
- ✅ Dynamic scheduling for edges
- Note: Memory-bound, limited speedup

### Task 5: Vector Addition
- ✅ Static scheduling (uniform work)
- ✅ Use large vectors (>10M elements)
- Note: Memory bandwidth limited

### Task 6: Sparse Matrix
- ✅ Dynamic scheduling (irregular work)
- ✅ Adaptive chunk size
- Note: Row-wise independence

---

## 🔗 Additional Resources

### OpenMP Documentation
- [OpenMP API Specification](https://www.openmp.org/specifications/)
- [OpenMP Best Practices](https://www.openmp.org/resources/tutorials-articles/)

### Performance Tools
- `time` - Basic timing
- `perf` - Linux performance profiler
- Intel VTune - Advanced profiling
- Valgrind Callgrind - Call graph profiling

### Books
- "Using OpenMP" by Chapman, Jost, and van der Pas
- "Parallel Programming in OpenMP" by Chandra et al.

---

## ✨ Final Checklist

Before submitting your parallel code:

- [ ] Correctness verified against sequential version
- [ ] No atomic/critical in hot loops
- [ ] Proper work decomposition (independent units)
- [ ] Appropriate scheduling policy chosen
- [ ] Tested with multiple problem sizes
- [ ] Documented performance characteristics
- [ ] Measured and reported speedup
- [ ] Code is clean and well-commented

---

**Remember:** Good parallel code requires thinking about the algorithm, not just adding `#pragma omp parallel`. Understanding data dependencies and work decomposition is essential!

