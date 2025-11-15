/*
 * Task 6: Parallel Sparse Matrix-Vector Multiplication
 * 
 * Description:
 *   Implements sparse matrix-vector multiplication using CSR format.
 *   Assigns row blocks to threads and handles irregular workload balancing.
 * 
 * Compilation: gcc -fopenmp -o sparse_matrix_vector.exe sparse_matrix_vector.c -lm
 * Usage: ./sparse_matrix_vector.exe [num_rows] [density]
 * 
 * Author: High Performance Computing Course
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <string.h>

// SpMV is memory-bound with irregular access patterns
// For good parallel speedup, use larger matrices (50K+ rows)
// Dynamic scheduling handles load imbalance from irregular sparsity
#define DEFAULT_ROWS 50000        // Increased from 10K for better parallelization
#define DEFAULT_DENSITY 0.05      // 5% non-zero elements

// CSR (Compressed Sparse Row) format
typedef struct {
    int num_rows;
    int num_cols;
    int nnz;              // Number of non-zero elements
    double *values;       // Non-zero values
    int *col_indices;     // Column index for each value
    int *row_ptr;         // Row pointer array
} CSRMatrix;

// Function prototypes
CSRMatrix* create_random_sparse_matrix(int rows, int cols, double density);
void free_csr_matrix(CSRMatrix *matrix);
void print_csr_format(CSRMatrix *matrix);
void spmv_sequential(CSRMatrix *A, double *x, double *y);
void spmv_parallel_static(CSRMatrix *A, double *x, double *y);
void spmv_parallel_dynamic(CSRMatrix *A, double *x, double *y);
int verify_results(double *y1, double *y2, int size);
void print_vector(double *vec, int size, const char *name);

int main(int argc, char *argv[]) {
    int num_rows = DEFAULT_ROWS;
    double density = DEFAULT_DENSITY;
    
    if (argc > 1) num_rows = atoi(argv[1]);
    if (argc > 2) density = atof(argv[2]);
    
    int num_cols = num_rows;  // Square matrix
    
    printf("==============================================\n");
    printf("  SPARSE MATRIX-VECTOR MULTIPLICATION (CSR)  \n");
    printf("==============================================\n");
    printf("Matrix size: %d x %d\n", num_rows, num_cols);
    printf("Density: %.2f%%\n", density * 100);
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("==============================================\n\n");
    
    // Create sparse matrix in CSR format
    printf("Generating random sparse matrix...\n");
    CSRMatrix *A = create_random_sparse_matrix(num_rows, num_cols, density);
    printf("Matrix created: %d non-zero elements out of %d (%.2f%%)\n",
           A->nnz, num_rows * num_cols, (A->nnz * 100.0) / (num_rows * num_cols));
    printf("Memory saved: %.2f MB (vs %.2f MB for dense)\n",
           (A->nnz * (sizeof(double) + sizeof(int)) + (num_rows + 1) * sizeof(int)) / (1024.0 * 1024.0),
           (num_rows * num_cols * sizeof(double)) / (1024.0 * 1024.0));
    
    // Print CSR format for small matrices
    if (num_rows <= 10) {
        print_csr_format(A);
    }
    
    // Allocate input vector x and output vector y
    double *x = (double *)malloc(num_cols * sizeof(double));
    double *y_seq = (double *)calloc(num_rows, sizeof(double));
    double *y_static = (double *)calloc(num_rows, sizeof(double));
    double *y_dynamic = (double *)calloc(num_rows, sizeof(double));
    
    if (!x || !y_seq || !y_static || !y_dynamic) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }
    
    // Initialize input vector x
    for (int i = 0; i < num_cols; i++) {
        x[i] = 1.0;  // Simple initialization
    }
    
    if (num_cols <= 10) {
        print_vector(x, num_cols, "Input vector x");
    }
    
    // Sequential SpMV
    printf("\n[1] Running SEQUENTIAL SpMV...\n");
    double start_seq = omp_get_wtime();
    spmv_sequential(A, x, y_seq);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    printf("    Time: %.6f seconds\n", time_seq);
    printf("    GFLOPS: %.3f\n", (2.0 * A->nnz / 1e9) / time_seq);
    
    // Parallel SpMV with static scheduling
    printf("\n[2] Running PARALLEL SpMV (STATIC)...\n");
    double start_static = omp_get_wtime();
    spmv_parallel_static(A, x, y_static);
    double end_static = omp_get_wtime();
    double time_static = end_static - start_static;
    printf("    Time: %.6f seconds\n", time_static);
    printf("    GFLOPS: %.3f\n", (2.0 * A->nnz / 1e9) / time_static);
    
    // Parallel SpMV with dynamic scheduling
    printf("\n[3] Running PARALLEL SpMV (DYNAMIC)...\n");
    double start_dynamic = omp_get_wtime();
    spmv_parallel_dynamic(A, x, y_dynamic);
    double end_dynamic = omp_get_wtime();
    double time_dynamic = end_dynamic - start_dynamic;
    printf("    Time: %.6f seconds\n", time_dynamic);
    printf("    GFLOPS: %.3f\n", (2.0 * A->nnz / 1e9) / time_dynamic);
    
    // Verify results
    printf("\n[4] Verifying results...\n");
    int static_correct = verify_results(y_seq, y_static, num_rows);
    int dynamic_correct = verify_results(y_seq, y_dynamic, num_rows);
    
    if (static_correct && dynamic_correct) {
        printf("    ✓ All results match! Correctness verified.\n");
    } else {
        printf("    ✗ Results differ! Check implementation.\n");
    }
    
    // Print result for small matrices
    if (num_rows <= 10) {
        print_vector(y_seq, num_rows, "Output vector y");
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
    printf("\n⚠️  PERFORMANCE CHARACTERISTICS:\n");
    printf("  • SpMV is MEMORY-BOUND with irregular access patterns\n");
    printf("  • Small matrices: Overhead dominates → may be slower parallel\n");
    printf("  • Dynamic scheduling handles irregular row workloads better\n");
    printf("\n");
    printf("  Performance scaling:\n");
    printf("  • Small (< 1K rows): Overhead may dominate, slower parallel\n");
    printf("  • Medium (1K-10K rows): Moderate speedup 2-4x\n");
    printf("  • Large (> 100K rows): Best speedup 4-8x with dynamic scheduling\n");
    printf("  • Static vs Dynamic: Dynamic better for power-law sparsity patterns\n");
    
    // Cleanup
    free_csr_matrix(A);
    free(x);
    free(y_seq);
    free(y_static);
    free(y_dynamic);
    
    return 0;
}

// Create random sparse matrix in CSR format
CSRMatrix* create_random_sparse_matrix(int rows, int cols, double density) {
    CSRMatrix *matrix = (CSRMatrix *)malloc(sizeof(CSRMatrix));
    matrix->num_rows = rows;
    matrix->num_cols = cols;
    
    // Estimate number of non-zeros
    int estimated_nnz = (int)(rows * cols * density);
    
    // Allocate temporary storage
    double *temp_values = (double *)malloc(estimated_nnz * 2 * sizeof(double));
    int *temp_cols = (int *)malloc(estimated_nnz * 2 * sizeof(int));
    matrix->row_ptr = (int *)malloc((rows + 1) * sizeof(int));
    
    srand(42);  // Fixed seed for reproducibility
    int nnz = 0;
    matrix->row_ptr[0] = 0;
    
    // Generate sparse matrix row by row
    for (int i = 0; i < rows; i++) {
        int row_nnz = 0;
        for (int j = 0; j < cols; j++) {
            double rand_val = (double)rand() / RAND_MAX;
            if (rand_val < density) {
                temp_values[nnz] = ((double)rand() / RAND_MAX) * 10.0;  // Random value 0-10
                temp_cols[nnz] = j;
                nnz++;
                row_nnz++;
            }
        }
        matrix->row_ptr[i + 1] = nnz;
    }
    
    // Allocate final storage
    matrix->nnz = nnz;
    matrix->values = (double *)malloc(nnz * sizeof(double));
    matrix->col_indices = (int *)malloc(nnz * sizeof(int));
    
    // Copy data
    memcpy(matrix->values, temp_values, nnz * sizeof(double));
    memcpy(matrix->col_indices, temp_cols, nnz * sizeof(int));
    
    free(temp_values);
    free(temp_cols);
    
    return matrix;
}

// Free CSR matrix
void free_csr_matrix(CSRMatrix *matrix) {
    if (matrix) {
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix->row_ptr);
        free(matrix);
    }
}

// Print CSR format
void print_csr_format(CSRMatrix *matrix) {
    printf("\nCSR Format Representation:\n");
    printf("---------------------------\n");
    printf("values:      [");
    for (int i = 0; i < matrix->nnz && i < 20; i++) {
        printf("%.1f", matrix->values[i]);
        if (i < matrix->nnz - 1) printf(", ");
    }
    if (matrix->nnz > 20) printf(", ...");
    printf("]\n");
    
    printf("col_indices: [");
    for (int i = 0; i < matrix->nnz && i < 20; i++) {
        printf("%d", matrix->col_indices[i]);
        if (i < matrix->nnz - 1) printf(", ");
    }
    if (matrix->nnz > 20) printf(", ...");
    printf("]\n");
    
    printf("row_ptr:     [");
    for (int i = 0; i <= matrix->num_rows && i < 20; i++) {
        printf("%d", matrix->row_ptr[i]);
        if (i < matrix->num_rows) printf(", ");
    }
    if (matrix->num_rows > 20) printf(", ...");
    printf("]\n");
    printf("---------------------------\n");
}

// Sequential sparse matrix-vector multiplication
void spmv_sequential(CSRMatrix *A, double *x, double *y) {
    for (int i = 0; i < A->num_rows; i++) {
        double sum = 0.0;
        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) {
            sum += A->values[j] * x[A->col_indices[j]];
        }
        y[i] = sum;
    }
}

// Parallel SpMV with static scheduling
void spmv_parallel_static(CSRMatrix *A, double *x, double *y) {
    /*
     * CORRECT IMPLEMENTATION: No synchronization needed
     * 
     * Key insight: Each row is computed independently
     * - Thread i computes y[rows assigned to i]
     * - Different threads write to different y[] elements
     * - NO shared writes = NO atomic/critical sections needed
     * 
     * Static scheduling:
     * - Good for uniform row workloads (regular sparsity)
     * - May have load imbalance for irregular patterns
     */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("    Using %d threads with STATIC scheduling\n", omp_get_num_threads());
        }
        
        // Parallelize over rows - each row is independent
        #pragma omp for schedule(static)
        for (int i = 0; i < A->num_rows; i++) {
            double sum = 0.0;
            for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) {
                sum += A->values[j] * x[A->col_indices[j]];
            }
            y[i] = sum;  // No race condition - this thread owns y[i]
        }
    }
}

// Parallel SpMV with dynamic scheduling
void spmv_parallel_dynamic(CSRMatrix *A, double *x, double *y) {
    /*
     * OPTIMIZED IMPLEMENTATION: Dynamic scheduling for load balancing
     * 
     * Why dynamic scheduling is better for sparse matrices:
     * - Rows can have vastly different numbers of non-zeros
     * - Static gives equal rows to each thread → unequal work
     * - Dynamic distributes work at runtime → better balance
     * 
     * Adaptive chunk size:
     * - Too small (1): High scheduling overhead
     * - Too large (1000s): Poor load balancing
     * - Optimal: ~10-100 rows per chunk
     */
    #pragma omp parallel
    {
        int num_threads;
        int chunk_size;
        
        #pragma omp single
        {
            num_threads = omp_get_num_threads();
            // Adaptive chunk size: aim for ~100 chunks per thread
            chunk_size = (A->num_rows / (num_threads * 100));
            if (chunk_size < 10) chunk_size = 10;
            if (chunk_size > 1000) chunk_size = 1000;
            
            printf("    Using %d threads with DYNAMIC scheduling (chunk=%d)\n", 
                   num_threads, chunk_size);
        }
        
        // Dynamic scheduling: runtime load balancing for irregular workloads
        // Each thread computes different rows - no synchronization needed
        #pragma omp for schedule(dynamic, chunk_size)
        for (int i = 0; i < A->num_rows; i++) {
            double sum = 0.0;
            for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) {
                sum += A->values[j] * x[A->col_indices[j]];
            }
            y[i] = sum;  // No race condition - exclusive ownership
        }
    }
}

// Verify results
int verify_results(double *y1, double *y2, int size) {
    double tolerance = 1e-9;
    int errors = 0;
    
    for (int i = 0; i < size; i++) {
        if (fabs(y1[i] - y2[i]) > tolerance) {
            errors++;
            if (errors <= 5) {
                printf("    Error at index %d: y1=%.6f, y2=%.6f\n", i, y1[i], y2[i]);
            }
        }
    }
    
    if (errors > 5) {
        printf("    ... and %d more errors\n", errors - 5);
    }
    
    return (errors == 0);
}

// Print vector
void print_vector(double *vec, int size, const char *name) {
    printf("\n%s: [", name);
    int print_count = (size < 10) ? size : 10;
    for (int i = 0; i < print_count; i++) {
        printf("%.1f", vec[i]);
        if (i < print_count - 1) printf(", ");
    }
    if (size > 10) printf(", ...");
    printf("]\n");
}

