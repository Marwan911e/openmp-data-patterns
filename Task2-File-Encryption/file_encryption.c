/*
 * Task 2: Parallel File Encryption
 * 
 * Description:
 *   Splits a large binary file into chunks and encrypts each chunk in parallel.
 *   Uses XOR encryption with synchronization to preserve output order.
 * 
 * Compilation: gcc -fopenmp -o file_encryption.exe file_encryption.c
 * Usage: ./file_encryption.exe [input_file] [output_file] [key]
 * 
 * Author: High Performance Computing Course
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define DEFAULT_CHUNK_SIZE (1024 * 1024)  // 1 MB per chunk
#define DEFAULT_KEY 0xA5                   // Default XOR key

// Function prototypes
long get_file_size(const char *filename);
void generate_test_file(const char *filename, long size);
void encrypt_sequential(const char *input, const char *output, unsigned char key);
void encrypt_parallel(const char *input, const char *output, unsigned char key, int chunk_size);
int verify_encryption(const char *original, const char *encrypted, unsigned char key);
void print_hex_sample(unsigned char *data, int size, const char *label);

int main(int argc, char *argv[]) {
    const char *input_file = "test_input.bin";
    const char *output_seq = "output_sequential.bin";
    const char *output_par = "output_parallel.bin";
    unsigned char key = DEFAULT_KEY;
    int chunk_size = DEFAULT_CHUNK_SIZE;
    
    if (argc > 1) input_file = argv[1];
    if (argc > 2) output_par = argv[2];
    if (argc > 3) key = (unsigned char)atoi(argv[3]);
    
    printf("==============================================\n");
    printf("       PARALLEL FILE ENCRYPTION (XOR)        \n");
    printf("==============================================\n");
    printf("Input file: %s\n", input_file);
    printf("Output file: %s\n", output_par);
    printf("Encryption key: 0x%02X\n", key);
    printf("Chunk size: %d bytes (%.2f MB)\n", chunk_size, chunk_size / (1024.0 * 1024.0));
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("==============================================\n\n");
    
    // Check if input file exists, create test file if not
    FILE *test = fopen(input_file, "rb");
    if (!test) {
        printf("Input file not found. Generating test file (10 MB)...\n");
        generate_test_file(input_file, 10 * 1024 * 1024);
    } else {
        fclose(test);
    }
    
    long file_size = get_file_size(input_file);
    printf("File size: %ld bytes (%.2f MB)\n", file_size, file_size / (1024.0 * 1024.0));
    printf("Number of chunks: %d\n\n", (int)((file_size + chunk_size - 1) / chunk_size));
    
    // Sequential encryption
    printf("[1] Running SEQUENTIAL encryption...\n");
    double start_seq = omp_get_wtime();
    encrypt_sequential(input_file, output_seq, key);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    printf("    Time: %.6f seconds\n", time_seq);
    printf("    Throughput: %.2f MB/s\n", (file_size / (1024.0 * 1024.0)) / time_seq);
    
    // Parallel encryption
    printf("\n[2] Running PARALLEL encryption...\n");
    double start_par = omp_get_wtime();
    encrypt_parallel(input_file, output_par, key, chunk_size);
    double end_par = omp_get_wtime();
    double time_par = end_par - start_par;
    printf("    Time: %.6f seconds\n", time_par);
    printf("    Throughput: %.2f MB/s\n", (file_size / (1024.0 * 1024.0)) / time_par);
    
    // Verify results
    printf("\n[3] Verifying encryption correctness...\n");
    int correct = verify_encryption(input_file, output_par, key);
    if (correct) {
        printf("    ✓ Encryption verified! Sequential and parallel outputs match.\n");
    } else {
        printf("    ✗ Error: Outputs differ!\n");
    }
    
    // Performance summary
    printf("\n==============================================\n");
    printf("  PERFORMANCE SUMMARY\n");
    printf("==============================================\n");
    printf("Sequential time:   %.6f seconds\n", time_seq);
    printf("Parallel time:     %.6f seconds\n", time_par);
    printf("Speedup:           %.2fx\n", time_seq / time_par);
    printf("Efficiency:        %.1f%%\n", (time_seq / time_par) / omp_get_max_threads() * 100);
    printf("==============================================\n");
    
    return 0;
}

// Get file size
long get_file_size(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

// Generate test file with pseudo-random data
void generate_test_file(const char *filename, long size) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to create test file!\n");
        exit(1);
    }
    
    unsigned char buffer[4096];
    long remaining = size;
    srand(42);
    
    while (remaining > 0) {
        int write_size = (remaining < 4096) ? remaining : 4096;
        for (int i = 0; i < write_size; i++) {
            buffer[i] = rand() % 256;
        }
        fwrite(buffer, 1, write_size, fp);
        remaining -= write_size;
    }
    
    fclose(fp);
    printf("Test file created: %s (%.2f MB)\n", filename, size / (1024.0 * 1024.0));
}

// Sequential file encryption
void encrypt_sequential(const char *input, const char *output, unsigned char key) {
    FILE *fin = fopen(input, "rb");
    FILE *fout = fopen(output, "wb");
    
    if (!fin || !fout) {
        fprintf(stderr, "Failed to open files for sequential encryption!\n");
        exit(1);
    }
    
    unsigned char buffer[4096];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, 4096, fin)) > 0) {
        // XOR encryption
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= key;
        }
        fwrite(buffer, 1, bytes_read, fout);
    }
    
    fclose(fin);
    fclose(fout);
}

// Parallel file encryption with chunk decomposition
void encrypt_parallel(const char *input, const char *output, unsigned char key, int chunk_size) {
    // Read entire file into memory
    long file_size = get_file_size(input);
    unsigned char *file_data = (unsigned char *)malloc(file_size);
    
    if (!file_data) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
    
    FILE *fin = fopen(input, "rb");
    if (!fin) {
        fprintf(stderr, "Failed to open input file!\n");
        free(file_data);
        exit(1);
    }
    
    fread(file_data, 1, file_size, fin);
    fclose(fin);
    
    // Calculate number of chunks
    int num_chunks = (file_size + chunk_size - 1) / chunk_size;
    
    // Parallel encryption of chunks
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp single
        {
            printf("    Using %d threads for chunk encryption\n", omp_get_num_threads());
            printf("    Processing %d chunks...\n", num_chunks);
        }
        
        #pragma omp for schedule(static)
        for (int chunk = 0; chunk < num_chunks; chunk++) {
            long start_pos = (long)chunk * chunk_size;
            long end_pos = start_pos + chunk_size;
            if (end_pos > file_size) end_pos = file_size;
            
            // Encrypt this chunk
            for (long i = start_pos; i < end_pos; i++) {
                file_data[i] ^= key;
            }
            
            // Progress indicator (every 10 chunks)
            if (chunk % 10 == 0) {
                #pragma omp critical
                {
                    printf("    Thread %d: Processed chunk %d/%d\n", 
                           thread_id, chunk + 1, num_chunks);
                }
            }
        }
    }
    
    // Write encrypted data (synchronized, single-threaded)
    FILE *fout = fopen(output, "wb");
    if (!fout) {
        fprintf(stderr, "Failed to open output file!\n");
        free(file_data);
        exit(1);
    }
    
    fwrite(file_data, 1, file_size, fout);
    fclose(fout);
    free(file_data);
}

// Verify encryption by comparing outputs
int verify_encryption(const char *original, const char *encrypted, unsigned char key) {
    FILE *fin = fopen(original, "rb");
    FILE *fenc = fopen(encrypted, "rb");
    
    if (!fin || !fenc) {
        fprintf(stderr, "Failed to open files for verification!\n");
        return 0;
    }
    
    unsigned char buf_orig, buf_enc;
    long pos = 0;
    int errors = 0;
    
    while (fread(&buf_orig, 1, 1, fin) == 1) {
        if (fread(&buf_enc, 1, 1, fenc) != 1) {
            printf("    Error: File size mismatch!\n");
            errors++;
            break;
        }
        
        // Encrypted byte should be original XOR key
        if (buf_enc != (buf_orig ^ key)) {
            errors++;
            if (errors <= 5) {
                printf("    Error at position %ld: expected 0x%02X, got 0x%02X\n",
                       pos, buf_orig ^ key, buf_enc);
            }
        }
        pos++;
    }
    
    fclose(fin);
    fclose(fenc);
    
    return (errors == 0);
}

// Print hex sample of data
void print_hex_sample(unsigned char *data, int size, const char *label) {
    printf("%s (first 32 bytes):\n", label);
    for (int i = 0; i < 32 && i < size; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

