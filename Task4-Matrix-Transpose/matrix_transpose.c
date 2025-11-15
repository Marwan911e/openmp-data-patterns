/*
 * Task 4: Matrix Transpose (Block Data Partition)
 * 
 * Description:
 *   Implements parallel matrix transpose using block-based decomposition.
 *   Divides the matrix into smaller sub-blocks for cache efficiency.
 * 
 * Compilation: gcc -fopenmp -o matrix_transpose.exe matrix_transpose.c -lm
 * Usage: ./matrix_transpose.exe [matrix_size] [block_size]
 * 
 * Author: High Performance Computing Course
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

// Note: Transpose is memory-bound. For good speedup, use large matrices
// Small matrices have parallel overhead > computation time
#define DEFAULT_SIZE 4096       // Increased from 2048 for better parallelization
#define DEFAULT_BLOCK_SIZE 64

// Function prototypes
void initialize_matrix(double *matrix, int rows, int cols, int seed);
void transpose_sequential(double *A, double *B, int N);
void transpose_parallel_naive(double *A, double *B, int N);
void transpose_parallel_blocked(double *A, double *B, int N, int block_size);
void print_matrix(double *matrix, int rows, int cols, int max_print);
int verify_transpose(double *A, double *B, int N);

int main(int argc, char *argv[]) {
    int N = DEFAULT_SIZE;
    int block_size = DEFAULT_BLOCK_SIZE;
    
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) block_size = atoi(argv[2]);
    
    printf("==============================================\n");
    printf("    PARALLEL MATRIX TRANSPOSE (BLOCKED)      \n");
    printf("==============================================\n");
    printf("Matrix Size: %d x %d\n", N, N);
    printf("Block Size: %d x %d\n", block_size, block_size);
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("==============================================\n\n");
    
    // Allocate matrices
    double *A = (double *)malloc(N * N * sizeof(double));
    double *B_seq = (double *)malloc(N * N * sizeof(double));
    double *B_naive = (double *)malloc(N * N * sizeof(double));
    double *B_blocked = (double *)malloc(N * N * sizeof(double));
    
    if (!A || !B_seq || !B_naive || !B_blocked) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }
    
    // Initialize matrix
    printf("Initializing matrix...\n");
    initialize_matrix(A, N, N, 42);
    
    // Print small sample
    if (N <= 8) {
        printf("\nOriginal Matrix A:\n");
        print_matrix(A, N, N, N);
    }
    
    // Sequential transpose
    printf("\n[1] Running SEQUENTIAL transpose...\n");
    double start_seq = omp_get_wtime();
    transpose_sequential(A, B_seq, N);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    printf("    Time: %.6f seconds\n", time_seq);
    
    // Parallel naive transpose
    printf("\n[2] Running PARALLEL NAIVE transpose...\n");
    double start_naive = omp_get_wtime();
    transpose_parallel_naive(A, B_naive, N);
    double end_naive = omp_get_wtime();
    double time_naive = end_naive - start_naive;
    printf("    Time: %.6f seconds\n", time_naive);
    
    // Parallel blocked transpose
    printf("\n[3] Running PARALLEL BLOCKED transpose...\n");
    double start_blocked = omp_get_wtime();
    transpose_parallel_blocked(A, B_blocked, N, block_size);
    double end_blocked = omp_get_wtime();
    double time_blocked = end_blocked - start_blocked;
    printf("    Time: %.6f seconds\n", time_blocked);
    
    // Verify results
    printf("\n[4] Verifying results...\n");
    int naive_correct = verify_transpose(A, B_naive, N);
    int blocked_correct = verify_transpose(A, B_blocked, N);
    
    if (naive_correct && blocked_correct) {
        printf("    ✓ All transposes correct!\n");
    } else {
        printf("    ✗ Error in transpose implementation!\n");
    }
    
    // Print result if small
    if (N <= 8) {
        printf("\nTransposed Matrix B:\n");
        print_matrix(B_blocked, N, N, N);
    }
    
    // Performance summary
    printf("\n==============================================\n");
    printf("  PERFORMANCE SUMMARY\n");
    printf("==============================================\n");
    printf("Sequential time:      %.6f seconds\n", time_seq);
    printf("Parallel (naive):     %.6f seconds (%.2fx speedup, %.1f%% eff.)\n", 
           time_naive, time_seq / time_naive,
           (time_seq / time_naive) / omp_get_max_threads() * 100);
    printf("Parallel (blocked):   %.6f seconds (%.2fx speedup, %.1f%% eff.)\n", 
           time_blocked, time_seq / time_blocked,
           (time_seq / time_blocked) / omp_get_max_threads() * 100);
    printf("==============================================\n");
    printf("\n⚠️  PERFORMANCE NOTES:\n");
    printf("  • Matrix transpose is MEMORY-BOUND (not compute-bound)\n");
    printf("  • Strided writes (B[j*N+i]) cause cache misses\n");
    printf("  • Blocked version improves cache locality\n");
    printf("  • Expected speedup: 4-8x with large matrices\n");
    printf("  • Small matrices: Overhead may dominate → slower parallel\n");
    
    // Cleanup
    free(A);
    free(B_seq);
    free(B_naive);
    free(B_blocked);
    
    return 0;
}

// Initialize matrix with values
void initialize_matrix(double *matrix, int rows, int cols, int seed) {
    srand(seed);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = (double)(i * cols + j);  // Sequential values for easy verification
        }
    }
}

// Sequential matrix transpose
void transpose_sequential(double *A, double *B, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            B[j * N + i] = A[i * N + j];
        }
    }
}

// Parallel naive transpose (simple parallelization)
void transpose_parallel_naive(double *A, double *B, int N) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("    Using %d threads (naive approach)\n", omp_get_num_threads());
        }
        
        #pragma omp for collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                B[j * N + i] = A[i * N + j];
            }
        }
    }
}

// Parallel blocked transpose (cache-efficient)
void transpose_parallel_blocked(double *A, double *B, int N, int block_size) {
    /*
     * CORRECT IMPLEMENTATION: No synchronization needed
     * 
     * Key insight: Each output element B[j][i] is written exactly once
     * by the thread that processes block (bi, bj) containing that element.
     * 
     * No race conditions because:
     * - Block (bi, bj) owns output elements B[bj:bj+block][bi:bi+block]
     * - Different blocks write to disjoint memory regions
     * - NO shared writes = NO atomic/critical sections needed
     */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("    Using %d threads (blocked approach, block=%dx%d)\n", 
                   omp_get_num_threads(), block_size, block_size);
        }
        
        // Parallelize over blocks - each block is independent
        // Dynamic scheduling handles edge blocks and load imbalances better
        #pragma omp for collapse(2) schedule(dynamic)
        for (int bi = 0; bi < N; bi += block_size) {
            for (int bj = 0; bj < N; bj += block_size) {
                // Compute block boundaries
                int i_end = (bi + block_size < N) ? bi + block_size : N;
                int j_end = (bj + block_size < N) ? bj + block_size : N;
                
                // Transpose this block
                // Each B[j*N+i] is written exactly once - no conflicts
                for (int i = bi; i < i_end; i++) {
                    for (int j = bj; j < j_end; j++) {
                        B[j * N + i] = A[i * N + j];
                    }
                }
            }
        }
    }
}

// Print matrix
void print_matrix(double *matrix, int rows, int cols, int max_print) {
    int row_limit = (rows < max_print) ? rows : max_print;
    int col_limit = (cols < max_print) ? cols : max_print;
    
    for (int i = 0; i < row_limit; i++) {
        for (int j = 0; j < col_limit; j++) {
            printf("%6.0f ", matrix[i * cols + j]);
        }
        if (col_limit < cols) printf("...");
        printf("\n");
    }
    if (row_limit < rows) {
        printf("...\n");
    }
}

// Verify transpose: B[j][i] should equal A[i][j]
int verify_transpose(double *A, double *B, int N) {
    int errors = 0;
    double tolerance = 1e-9;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fabs(B[j * N + i] - A[i * N + j]) > tolerance) {
                errors++;
                if (errors <= 5) {
                    printf("    Error at (%d,%d): A[%d][%d]=%.2f, B[%d][%d]=%.2f\n",
                           i, j, i, j, A[i * N + j], j, i, B[j * N + i]);
                }
            }
        }
    }
    
    if (errors > 5) {
        printf("    ... and %d more errors\n", errors - 5);
    }
    
    return (errors == 0);
}

