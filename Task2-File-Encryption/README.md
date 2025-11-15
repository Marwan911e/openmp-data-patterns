# Task 2: Parallel File Encryption

## Overview

This task implements parallel file encryption using **chunk-based data decomposition**. The file is split into equal-sized chunks, and each chunk is encrypted in parallel using XOR cipher. Output writes are synchronized to preserve the correct order.

## Algorithm

### Chunk-Based Decomposition

1. **Read** the entire file into memory
2. **Divide** file into chunks of fixed size (e.g., 1 MB)
3. **Assign** each chunk to a thread for parallel encryption
4. **Encrypt** using XOR cipher: `encrypted_byte = original_byte ^ key`
5. **Write** the encrypted data back to output file

### XOR Encryption

Simple symmetric cipher where encryption and decryption use the same operation:

```
Encryption: C = P ⊕ K
Decryption: P = C ⊕ K
```

Where:
- P = Plaintext
- C = Ciphertext
- K = Key
- ⊕ = XOR operation

## Implementation Details

### Key OpenMP Features

- `#pragma omp parallel for schedule(static)` - Static chunk assignment
- `#pragma omp critical` - Thread-safe progress reporting
- Static scheduling ensures predictable chunk distribution

### Data Decomposition

```
File (N bytes) → Chunks (chunk_size bytes each)

Example: 10 MB file with 1 MB chunks = 10 chunks
Each thread processes assigned chunks independently
```

### Synchronization Strategy

- **Encryption phase:** No synchronization needed (independent chunks)
- **I/O phase:** Sequential write after all chunks encrypted
- **Progress reporting:** Critical section for console output

## Compilation

```bash
gcc -fopenmp -o file_encryption.exe file_encryption.c
```

## Usage

```bash
# Default (generates 10 MB test file)
./file_encryption.exe

# Encrypt specific file
./file_encryption.exe input.bin output.bin

# Custom encryption key
./file_encryption.exe input.bin output.bin 0xAB

# Arguments: [input_file] [output_file] [key]
```

## Example Output

```
==============================================
       PARALLEL FILE ENCRYPTION (XOR)        
==============================================
Input file: test_input.bin
Output file: output_parallel.bin
Encryption key: 0xA5
Chunk size: 1048576 bytes (1.00 MB)
Number of threads: 8
==============================================

File size: 10485760 bytes (10.00 MB)
Number of chunks: 10

[1] Running SEQUENTIAL encryption...
    Time: 0.045231 seconds
    Throughput: 221.12 MB/s

[2] Running PARALLEL encryption...
    Using 8 threads for chunk encryption
    Processing 10 chunks...
    Thread 2: Processed chunk 1/10
    Thread 5: Processed chunk 6/10
    Time: 0.008452 seconds
    Throughput: 1183.45 MB/s

[3] Verifying encryption correctness...
    ✓ Encryption verified! Sequential and parallel outputs match.

==============================================
  PERFORMANCE SUMMARY
==============================================
Sequential time:   0.045231 seconds
Parallel time:     0.008452 seconds
Speedup:           5.35x
Efficiency:        66.9%
==============================================
```

## Performance Analysis

### Time Complexity

- **Sequential:** O(N) where N = file size
- **Parallel:** O(N / P) where P = number of threads

### Expected Speedup

| File Size | Threads | Speedup |
|-----------|---------|---------|
| 10 MB     | 4       | ~3.5x   |
| 100 MB    | 8       | ~6.0x   |
| 1 GB      | 16      | ~11x    |

### Throughput

Parallel encryption can achieve significantly higher throughput:
- Sequential: ~200-300 MB/s
- Parallel (8 threads): ~1000-1500 MB/s

## Advantages of Chunk Decomposition

1. **Simplicity:** Easy to implement and debug
2. **Load Balancing:** Equal-sized chunks distribute work evenly
3. **Scalability:** More threads = more chunks processed simultaneously
4. **Memory Efficiency:** Can be adapted for streaming (chunk-at-a-time)

## Security Considerations

### XOR Cipher

- **Simple but weak:** Not suitable for real-world encryption
- **Educational purpose:** Demonstrates parallel encryption concept
- **Key reuse:** Using the same key for entire file is vulnerable

### Production Encryption

For real applications, use:
- **AES** (Advanced Encryption Standard)
- **ChaCha20** stream cipher
- Proper **key management** and **initialization vectors**

## Key Concepts

### 1. Data Decomposition

Breaking the file into independent chunks that can be processed in parallel.

### 2. Static Scheduling

```c
#pragma omp for schedule(static)
```

Each thread gets a contiguous block of chunks, predictable distribution.

### 3. Memory-Bound Operation

Encryption performance often limited by memory bandwidth, not CPU.

### 4. I/O Synchronization

Reading/writing must preserve order to maintain file integrity.

## Testing

### Small File Test
```bash
# Create small test
echo "Hello, World!" > test.txt
./file_encryption.exe test.txt encrypted.txt 0x42

# Decrypt (XOR twice returns original)
./file_encryption.exe encrypted.txt decrypted.txt 0x42
diff test.txt decrypted.txt
```

### Large File Test
```bash
# Generate 100 MB test file
dd if=/dev/urandom of=large_test.bin bs=1M count=100
./file_encryption.exe large_test.bin large_encrypted.bin
```

## Extensions

### Streaming Encryption

Modify to process one chunk at a time without loading entire file:

```c
// Read chunk → Encrypt → Write → Next chunk
```

### AES Encryption

Replace XOR with AES cipher:

```c
// Use OpenSSL library for AES
#include <openssl/aes.h>
```

### Ordered Writing

Use `#pragma omp ordered` to write chunks in order during encryption.

## Common Issues

1. **Memory exhaustion?**
   - File too large for RAM
   - Switch to streaming mode

2. **Poor speedup?**
   - File too small (overhead dominates)
   - I/O bottleneck (use SSD)
   - Increase file size

3. **Wrong output?**
   - Check chunk boundaries
   - Verify synchronization in writes

## References

- XOR cipher: https://en.wikipedia.org/wiki/XOR_cipher
- OpenMP scheduling: https://www.openmp.org/spec-html/5.0/openmpsu42.html
- AES encryption: https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
- File I/O in C: https://www.cplusplus.com/reference/cstdio/

