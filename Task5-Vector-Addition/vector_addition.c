/*
 * Task 5: Vector Addition (Element Partitioning)
 * 
 * Description:
 *   Implements parallel addition of two large vectors.
 *   Partitions elements evenly among threads using OpenMP.
 * 
 * Compilation: gcc -fopenmp -o vector_addition.exe vector_addition.c
 * Usage: ./vector_addition.exe [vector_size]
 * 
 * Author: High Performance Computing Course
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

// Vector addition is MEMORY-BOUND, not CPU-bound
// Performance limited by memory bandwidth, not computation
// Speedup saturates when memory bandwidth is fully utilized
// For testing: Use large vectors (100M+ elements) to amortize parallel overhead
#define DEFAULT_SIZE 100000000  // 100 million elements

// Function prototypes
void initialize_vector(double *vec, int size, double value);
void vector_add_sequential(double *A, double *B, double *C, int size);
void vector_add_parallel_static(double *A, double *B, double *C, int size);
void vector_add_parallel_dynamic(double *A, double *B, double *C, int size);
int verify_results(double *C1, double *C2, int size);
void print_vector_sample(double *vec, int size, const char *name);

int main(int argc, char *argv[]) {
    int size = DEFAULT_SIZE;
    
    if (argc > 1) size = atoi(argv[1]);
    
    printf("==============================================\n");
    printf("     PARALLEL VECTOR ADDITION (ELEMENT)      \n");
    printf("==============================================\n");
    printf("Vector size: %d elements\n", size);
    printf("Memory: %.2f MB per vector\n", (size * sizeof(double)) / (1024.0 * 1024.0));
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("==============================================\n\n");
    
    // Allocate vectors
    double *A = (double *)malloc(size * sizeof(double));
    double *B = (double *)malloc(size * sizeof(double));
    double *C_seq = (double *)malloc(size * sizeof(double));
    double *C_static = (double *)malloc(size * sizeof(double));
    double *C_dynamic = (double *)malloc(size * sizeof(double));
    
    if (!A || !B || !C_seq || !C_static || !C_dynamic) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }
    
    // Initialize vectors
    printf("Initializing vectors...\n");
    initialize_vector(A, size, 1.0);
    initialize_vector(B, size, 2.0);
    
    // Print samples
    if (size <= 20) {
        print_vector_sample(A, size, "Vector A");
        print_vector_sample(B, size, "Vector B");
    }
    
    // Sequential addition
    printf("\n[1] Running SEQUENTIAL vector addition...\n");
    double start_seq = omp_get_wtime();
    vector_add_sequential(A, B, C_seq, size);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    printf("    Time: %.6f seconds\n", time_seq);
    printf("    Throughput: %.2f Million ops/sec\n", (size / 1e6) / time_seq);
    
    // Parallel static scheduling
    printf("\n[2] Running PARALLEL vector addition (STATIC)...\n");
    double start_static = omp_get_wtime();
    vector_add_parallel_static(A, B, C_static, size);
    double end_static = omp_get_wtime();
    double time_static = end_static - start_static;
    printf("    Time: %.6f seconds\n", time_static);
    printf("    Throughput: %.2f Million ops/sec\n", (size / 1e6) / time_static);
    
    // Parallel dynamic scheduling
    printf("\n[3] Running PARALLEL vector addition (DYNAMIC)...\n");
    double start_dynamic = omp_get_wtime();
    vector_add_parallel_dynamic(A, B, C_dynamic, size);
    double end_dynamic = omp_get_wtime();
    double time_dynamic = end_dynamic - start_dynamic;
    printf("    Time: %.6f seconds\n", time_dynamic);
    printf("    Throughput: %.2f Million ops/sec\n", (size / 1e6) / time_dynamic);
    
    // Verify results
    printf("\n[4] Verifying results...\n");
    int static_correct = verify_results(C_seq, C_static, size);
    int dynamic_correct = verify_results(C_seq, C_dynamic, size);
    
    if (static_correct && dynamic_correct) {
        printf("    ✓ All results match! Correctness verified.\n");
    } else {
        printf("    ✗ Results differ! Check implementation.\n");
    }
    
    // Print result samples
    if (size <= 20) {
        print_vector_sample(C_static, size, "Result C");
    }
    
    // Performance summary
    printf("\n==============================================\n");
    printf("  PERFORMANCE SUMMARY\n");
    printf("==============================================\n");
    printf("Sequential time:     %.6f seconds\n", time_seq);
    printf("Parallel (static):   %.6f seconds (%.2fx speedup, %.1f%% eff.)\n", 
           time_static, time_seq / time_static,
           (time_seq / time_static) / omp_get_max_threads() * 100);
    printf("Parallel (dynamic):  %.6f seconds (%.2fx speedup, %.1f%% eff.)\n", 
           time_dynamic, time_seq / time_dynamic,
           (time_seq / time_dynamic) / omp_get_max_threads() * 100);
    printf("==============================================\n");
    printf("\n⚠️  CRITICAL PERFORMANCE CHARACTERISTICS:\n");
    printf("  • Vector addition is MEMORY-BOUND, not compute-bound\n");
    printf("  • Arithmetic intensity: 1 FLOP / 3 memory accesses (very low)\n");
    printf("  • Bottleneck: Memory bandwidth, NOT CPU computation\n");
    printf("  • Parallel overhead can dominate for small vectors\n");
    printf("\n");
    printf("  Expected behavior:\n");
    printf("  • Small vectors (< 1M): May be SLOWER parallel (overhead)\n");
    printf("  • Medium vectors (1-10M): Modest speedup 2-4x\n");
    printf("  • Large vectors (> 100M): Best speedup (up to memory bandwidth limit)\n");
    printf("  • Speedup ceiling: ~4-8x on typical systems (memory channels)\n");
    
    // Cleanup
    free(A);
    free(B);
    free(C_seq);
    free(C_static);
    free(C_dynamic);
    
    return 0;
}

// Initialize vector with a constant value
void initialize_vector(double *vec, int size, double value) {
    for (int i = 0; i < size; i++) {
        vec[i] = value;
    }
}

// Sequential vector addition
void vector_add_sequential(double *A, double *B, double *C, int size) {
    for (int i = 0; i < size; i++) {
        C[i] = A[i] + B[i];
    }
}

// Parallel vector addition with static scheduling
void vector_add_parallel_static(double *A, double *B, double *C, int size) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        #pragma omp single
        {
            printf("    Using %d threads with STATIC scheduling\n", num_threads);
        }
        
        #pragma omp for schedule(static)
        for (int i = 0; i < size; i++) {
            C[i] = A[i] + B[i];
        }
        
        // Show work distribution
        #pragma omp single
        {
            int chunk_size = (size + num_threads - 1) / num_threads;
            printf("    Chunk size per thread: ~%d elements\n", chunk_size);
        }
    }
}

// Parallel vector addition with dynamic scheduling
void vector_add_parallel_dynamic(double *A, double *B, double *C, int size) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("    Using %d threads with DYNAMIC scheduling\n", omp_get_num_threads());
        }
        
        #pragma omp for schedule(dynamic, 10000)
        for (int i = 0; i < size; i++) {
            C[i] = A[i] + B[i];
        }
    }
}

// Verify that two vectors are equal
int verify_results(double *C1, double *C2, int size) {
    double tolerance = 1e-9;
    int errors = 0;
    
    for (int i = 0; i < size; i++) {
        if (fabs(C1[i] - C2[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("    Error at index %d: C1=%.6f, C2=%.6f\n", i, C1[i], C2[i]);
            }
        }
    }
    
    if (errors > 5) {
        printf("    ... and %d more errors\n", errors - 5);
    }
    
    return (errors == 0);
}

// Print vector sample
void print_vector_sample(double *vec, int size, const char *name) {
    printf("\n%s: [", name);
    int print_count = (size < 20) ? size : 20;
    for (int i = 0; i < print_count; i++) {
        printf("%.1f", vec[i]);
        if (i < print_count - 1) printf(", ");
    }
    if (size > 20) printf(", ...");
    printf("]\n");
}

