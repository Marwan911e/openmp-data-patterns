/*
 * Task 3: Parallel Histogram Computation
 * 
 * Description:
 *   Computes a histogram of integers (0-9) from a large array.
 *   Uses data partitioning among threads with proper synchronization.
 * 
 * Compilation: gcc -fopenmp -o histogram.exe histogram.c
 * Usage: ./histogram.exe [array_size]
 * 
 * Author: High Performance Computing Course
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define NUM_BINS 10
#define DEFAULT_SIZE 10000000  // 10 million elements

// Function prototypes
void generate_data(int *data, int size);
void histogram_sequential(int *data, int size, int *histogram);
void histogram_parallel_atomic(int *data, int size, int *histogram);
void histogram_parallel_reduction(int *data, int size, int *histogram);
void print_histogram(int *histogram, const char *title);
int verify_histograms(int *h1, int *h2);

int main(int argc, char *argv[]) {
    int size = DEFAULT_SIZE;
    
    if (argc > 1) size = atoi(argv[1]);
    
    printf("==============================================\n");
    printf("    PARALLEL HISTOGRAM COMPUTATION (0-9)     \n");
    printf("==============================================\n");
    printf("Array size: %d elements\n", size);
    printf("Number of bins: %d (0-9)\n", NUM_BINS);
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("==============================================\n\n");
    
    // Allocate data array
    int *data = (int *)malloc(size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }
    
    // Generate random data
    printf("Generating random data (0-9)...\n");
    generate_data(data, size);
    
    // Allocate histogram arrays
    int histogram_seq[NUM_BINS] = {0};
    int histogram_atomic[NUM_BINS] = {0};
    int histogram_reduction[NUM_BINS] = {0};
    
    // Sequential histogram
    printf("\n[1] Running SEQUENTIAL histogram...\n");
    double start_seq = omp_get_wtime();
    histogram_sequential(data, size, histogram_seq);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    printf("    Time: %.6f seconds\n", time_seq);
    print_histogram(histogram_seq, "Sequential Histogram");
    
    // Parallel histogram with atomic operations
    printf("\n[2] Running PARALLEL histogram (ATOMIC)...\n");
    double start_atomic = omp_get_wtime();
    histogram_parallel_atomic(data, size, histogram_atomic);
    double end_atomic = omp_get_wtime();
    double time_atomic = end_atomic - start_atomic;
    printf("    Time: %.6f seconds\n", time_atomic);
    print_histogram(histogram_atomic, "Parallel Histogram (Atomic)");
    
    // Parallel histogram with reduction
    printf("\n[3] Running PARALLEL histogram (REDUCTION)...\n");
    double start_reduction = omp_get_wtime();
    histogram_parallel_reduction(data, size, histogram_reduction);
    double end_reduction = omp_get_wtime();
    double time_reduction = end_reduction - start_reduction;
    printf("    Time: %.6f seconds\n", time_reduction);
    print_histogram(histogram_reduction, "Parallel Histogram (Reduction)");
    
    // Verify results
    printf("\n[4] Verifying results...\n");
    int atomic_correct = verify_histograms(histogram_seq, histogram_atomic);
    int reduction_correct = verify_histograms(histogram_seq, histogram_reduction);
    
    if (atomic_correct && reduction_correct) {
        printf("    ✓ All histograms match! Correctness verified.\n");
    } else {
        printf("    ✗ Histograms differ! Check implementation.\n");
    }
    
    // Performance summary
    printf("\n==============================================\n");
    printf("  PERFORMANCE SUMMARY\n");
    printf("==============================================\n");
    printf("Sequential time:      %.6f seconds\n", time_seq);
    printf("Parallel (atomic):    %.6f seconds (%.2fx speedup, %.1f%% efficiency)\n", 
           time_atomic, time_seq / time_atomic, 
           (time_seq / time_atomic) / omp_get_max_threads() * 100);
    printf("Parallel (reduction): %.6f seconds (%.2fx speedup, %.1f%% efficiency)\n", 
           time_reduction, time_seq / time_reduction,
           (time_seq / time_reduction) / omp_get_max_threads() * 100);
    printf("==============================================\n");
    printf("\n⚠️  IMPORTANT PERFORMANCE NOTES:\n");
    printf("  • Atomic version: Educational example showing contention problems\n");
    printf("    - Causes 100K+ atomic operations (one per element)\n");
    printf("    - Extreme contention on 10 bins → 100-1000x SLOWER\n");
    printf("  • Reduction version: CORRECT approach for production code\n");
    printf("    - Only 10×threads atomic operations total\n");
    printf("    - Expected speedup: 5-10x with proper implementation\n");
    
    // Cleanup
    free(data);
    
    return 0;
}

// Generate random data in range [0, 9]
void generate_data(int *data, int size) {
    srand(42);  // Fixed seed for reproducibility
    for (int i = 0; i < size; i++) {
        data[i] = rand() % NUM_BINS;
    }
}

// Sequential histogram computation
void histogram_sequential(int *data, int size, int *histogram) {
    // Initialize histogram to zero
    memset(histogram, 0, NUM_BINS * sizeof(int));
    
    // Count occurrences
    for (int i = 0; i < size; i++) {
        histogram[data[i]]++;
    }
}

// Parallel histogram with atomic operations
// WARNING: This implementation is SLOW due to excessive contention
// Atomic operations on every element cause cache line bouncing and serialization
// Expected performance: 100-1000x SLOWER than sequential for high contention
// Use histogram_parallel_reduction() for good performance
void histogram_parallel_atomic(int *data, int size, int *histogram) {
    // Initialize histogram to zero
    memset(histogram, 0, NUM_BINS * sizeof(int));
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("    Using %d threads with atomic operations\n", omp_get_num_threads());
            printf("    WARNING: Atomic version is demonstrably SLOW due to contention\n");
        }
        
        // Each thread processes its portion of data
        // PERFORMANCE ISSUE: Atomic on every iteration causes massive contention
        // With only 10 bins, all threads constantly compete for the same memory locations
        #pragma omp for
        for (int i = 0; i < size; i++) {
            // Atomic increment to avoid race conditions
            // This serializes execution when multiple threads access same bin
            #pragma omp atomic
            histogram[data[i]]++;
        }
    }
}

// Parallel histogram with local histograms and reduction
// CORRECT APPROACH: Each thread builds private histogram, then combines at end
// This minimizes synchronization overhead - only 10 bins need to be merged
void histogram_parallel_reduction(int *data, int size, int *histogram) {
    // Initialize global histogram to zero
    memset(histogram, 0, NUM_BINS * sizeof(int));
    
    #pragma omp parallel
    {
        // Each thread has its own local histogram - NO CONTENTION during counting
        int local_hist[NUM_BINS] = {0};
        
        #pragma omp single
        {
            printf("    Using %d threads with local histogram reduction\n", omp_get_num_threads());
        }
        
        // Phase 1: Each thread computes its local histogram independently
        // NO synchronization needed - each thread works on private data
        #pragma omp for nowait
        for (int i = 0; i < size; i++) {
            local_hist[data[i]]++;  // Fast - no atomic, no contention
        }
        
        // Phase 2: Combine local histograms into global histogram
        // Only NUM_BINS operations per thread (very few compared to data size)
        // Using critical section for the entire reduction is more efficient than
        // atomic on each bin since we're updating multiple related values
        #pragma omp critical
        {
            for (int bin = 0; bin < NUM_BINS; bin++) {
                histogram[bin] += local_hist[bin];
            }
        }
    }
}

// Print histogram in a nice format
void print_histogram(int *histogram, const char *title) {
    printf("\n    %s:\n", title);
    printf("    --------------------------------\n");
    
    // Find max count for scaling
    int max_count = 0;
    for (int i = 0; i < NUM_BINS; i++) {
        if (histogram[i] > max_count) max_count = histogram[i];
    }
    
    // Print bars
    for (int i = 0; i < NUM_BINS; i++) {
        int bar_length = (histogram[i] * 50) / max_count;
        if (bar_length == 0 && histogram[i] > 0) bar_length = 1;
        
        printf("    %d: %10d |", i, histogram[i]);
        for (int j = 0; j < bar_length; j++) {
            printf("█");
        }
        printf("\n");
    }
    printf("    --------------------------------\n");
    
    // Print statistics
    int total = 0;
    for (int i = 0; i < NUM_BINS; i++) {
        total += histogram[i];
    }
    printf("    Total: %d elements\n", total);
}

// Verify that two histograms are equal
int verify_histograms(int *h1, int *h2) {
    int errors = 0;
    for (int i = 0; i < NUM_BINS; i++) {
        if (h1[i] != h2[i]) {
            printf("    Error in bin %d: h1=%d, h2=%d\n", i, h1[i], h2[i]);
            errors++;
        }
    }
    return (errors == 0);
}

