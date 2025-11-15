# Makefile for Assignment 3 - Data Decomposition with OpenMP
# Course: CCS4210 - High Performance Computing
# Instructor: Dr. Hanan Hassan
# TA: Marwa Alazab

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -fopenmp -O2 -Wall
LDFLAGS = -fopenmp -lm

# Directories
TASK1_DIR = Task1-Matrix-Multiplication
TASK2_DIR = Task2-File-Encryption
TASK3_DIR = Task3-Histogram
TASK4_DIR = Task4-Matrix-Transpose
TASK5_DIR = Task5-Vector-Addition
TASK6_DIR = Task6-Sparse-Matrix

# Executables
TASK1_EXE = $(TASK1_DIR)/matrix_multiplication.exe
TASK2_EXE = $(TASK2_DIR)/file_encryption.exe
TASK3_EXE = $(TASK3_DIR)/histogram.exe
TASK4_EXE = $(TASK4_DIR)/matrix_transpose.exe
TASK5_EXE = $(TASK5_DIR)/vector_addition.exe
TASK6_EXE = $(TASK6_DIR)/sparse_matrix_vector.exe

# Source files
TASK1_SRC = $(TASK1_DIR)/matrix_multiplication.c
TASK2_SRC = $(TASK2_DIR)/file_encryption.c
TASK3_SRC = $(TASK3_DIR)/histogram.c
TASK4_SRC = $(TASK4_DIR)/matrix_transpose.c
TASK5_SRC = $(TASK5_DIR)/vector_addition.c
TASK6_SRC = $(TASK6_DIR)/sparse_matrix_vector.c

# All executables
ALL_EXES = $(TASK1_EXE) $(TASK2_EXE) $(TASK3_EXE) $(TASK4_EXE) $(TASK5_EXE) $(TASK6_EXE)

# Default target: build all
.PHONY: all
all: $(ALL_EXES)
	@echo ""
	@echo "=========================================="
	@echo "  All tasks compiled successfully!"
	@echo "=========================================="
	@echo "Available executables:"
	@echo "  1. $(TASK1_EXE)"
	@echo "  2. $(TASK2_EXE)"
	@echo "  3. $(TASK3_EXE)"
	@echo "  4. $(TASK4_EXE)"
	@echo "  5. $(TASK5_EXE)"
	@echo "  6. $(TASK6_EXE)"
	@echo "=========================================="

# Individual tasks
.PHONY: task1 task2 task3 task4 task5 task6

task1: $(TASK1_EXE)
	@echo "✓ Task 1 compiled: $(TASK1_EXE)"

task2: $(TASK2_EXE)
	@echo "✓ Task 2 compiled: $(TASK2_EXE)"

task3: $(TASK3_EXE)
	@echo "✓ Task 3 compiled: $(TASK3_EXE)"

task4: $(TASK4_EXE)
	@echo "✓ Task 4 compiled: $(TASK4_EXE)"

task5: $(TASK5_EXE)
	@echo "✓ Task 5 compiled: $(TASK5_EXE)"

task6: $(TASK6_EXE)
	@echo "✓ Task 6 compiled: $(TASK6_EXE)"

# Build rules
$(TASK1_EXE): $(TASK1_SRC)
	@echo "Compiling Task 1: Matrix Multiplication..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TASK2_EXE): $(TASK2_SRC)
	@echo "Compiling Task 2: File Encryption..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TASK3_EXE): $(TASK3_SRC)
	@echo "Compiling Task 3: Histogram..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TASK4_EXE): $(TASK4_SRC)
	@echo "Compiling Task 4: Matrix Transpose..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TASK5_EXE): $(TASK5_SRC)
	@echo "Compiling Task 5: Vector Addition..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TASK6_EXE): $(TASK6_SRC)
	@echo "Compiling Task 6: Sparse Matrix-Vector Multiplication..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Run targets
.PHONY: run-task1 run-task2 run-task3 run-task4 run-task5 run-task6

run-task1: $(TASK1_EXE)
	@echo "\n========== Running Task 1 =========="
	./$(TASK1_EXE)

run-task2: $(TASK2_EXE)
	@echo "\n========== Running Task 2 =========="
	./$(TASK2_EXE)

run-task3: $(TASK3_EXE)
	@echo "\n========== Running Task 3 =========="
	./$(TASK3_EXE)

run-task4: $(TASK4_EXE)
	@echo "\n========== Running Task 4 =========="
	./$(TASK4_EXE)

run-task5: $(TASK5_EXE)
	@echo "\n========== Running Task 5 =========="
	./$(TASK5_EXE)

run-task6: $(TASK6_EXE)
	@echo "\n========== Running Task 6 =========="
	./$(TASK6_EXE)

# Run all tasks
.PHONY: run-all
run-all: all
	@echo "\n=========================================="
	@echo "  Running all tasks sequentially..."
	@echo "=========================================="
	@$(MAKE) run-task1
	@$(MAKE) run-task2
	@$(MAKE) run-task3
	@$(MAKE) run-task4
	@$(MAKE) run-task5
	@$(MAKE) run-task6
	@echo "\n=========================================="
	@echo "  All tasks completed!"
	@echo "=========================================="

# Test targets (run with small inputs for quick verification)
.PHONY: test test-all

test-task1: $(TASK1_EXE)
	@echo "\n========== Testing Task 1 (small input) =========="
	./$(TASK1_EXE) 128 32

test-task2: $(TASK2_EXE)
	@echo "\n========== Testing Task 2 (small input) =========="
	./$(TASK2_EXE)

test-task3: $(TASK3_EXE)
	@echo "\n========== Testing Task 3 (small input) =========="
	./$(TASK3_EXE) 100000

test-task4: $(TASK4_EXE)
	@echo "\n========== Testing Task 4 (small input) =========="
	./$(TASK4_EXE) 512 64

test-task5: $(TASK5_EXE)
	@echo "\n========== Testing Task 5 (small input) =========="
	./$(TASK5_EXE) 1000000

test-task6: $(TASK6_EXE)
	@echo "\n========== Testing Task 6 (small input) =========="
	./$(TASK6_EXE) 1000 0.05

test-all: all
	@echo "\n=========================================="
	@echo "  Testing all tasks (quick verification)..."
	@echo "=========================================="
	@$(MAKE) test-task1
	@$(MAKE) test-task2
	@$(MAKE) test-task3
	@$(MAKE) test-task4
	@$(MAKE) test-task5
	@$(MAKE) test-task6
	@echo "\n=========================================="
	@echo "  All tests completed!"
	@echo "=========================================="

# Clean targets
.PHONY: clean clean-all clean-task1 clean-task2 clean-task3 clean-task4 clean-task5 clean-task6

clean-task1:
	@echo "Cleaning Task 1..."
	@rm -f $(TASK1_EXE)

clean-task2:
	@echo "Cleaning Task 2..."
	@rm -f $(TASK2_EXE)
	@rm -f test_input.bin output_*.bin

clean-task3:
	@echo "Cleaning Task 3..."
	@rm -f $(TASK3_EXE)

clean-task4:
	@echo "Cleaning Task 4..."
	@rm -f $(TASK4_EXE)

clean-task5:
	@echo "Cleaning Task 5..."
	@rm -f $(TASK5_EXE)

clean-task6:
	@echo "Cleaning Task 6..."
	@rm -f $(TASK6_EXE)

clean: clean-task1 clean-task2 clean-task3 clean-task4 clean-task5 clean-task6
	@echo "All executables cleaned."

clean-all: clean
	@echo "Removing all temporary files..."
	@rm -f *~ */*~
	@rm -f *.o */*.o
	@echo "Full cleanup completed."

# Help target
.PHONY: help
help:
	@echo "=========================================="
	@echo "  Assignment 3 - Data Decomposition"
	@echo "  Makefile Help"
	@echo "=========================================="
	@echo ""
	@echo "Build targets:"
	@echo "  make              - Build all tasks (default)"
	@echo "  make all          - Build all tasks"
	@echo "  make task1        - Build Task 1 (Matrix Multiplication)"
	@echo "  make task2        - Build Task 2 (File Encryption)"
	@echo "  make task3        - Build Task 3 (Histogram)"
	@echo "  make task4        - Build Task 4 (Matrix Transpose)"
	@echo "  make task5        - Build Task 5 (Vector Addition)"
	@echo "  make task6        - Build Task 6 (Sparse Matrix-Vector)"
	@echo ""
	@echo "Run targets:"
	@echo "  make run-task1    - Run Task 1"
	@echo "  make run-task2    - Run Task 2"
	@echo "  make run-task3    - Run Task 3"
	@echo "  make run-task4    - Run Task 4"
	@echo "  make run-task5    - Run Task 5"
	@echo "  make run-task6    - Run Task 6"
	@echo "  make run-all      - Run all tasks"
	@echo ""
	@echo "Test targets:"
	@echo "  make test-task1   - Test Task 1 (small input)"
	@echo "  make test-all     - Test all tasks (quick verification)"
	@echo ""
	@echo "Clean targets:"
	@echo "  make clean        - Remove all executables"
	@echo "  make clean-all    - Remove executables and temp files"
	@echo "  make clean-task1  - Clean Task 1 only"
	@echo ""
	@echo "Other targets:"
	@echo "  make help         - Show this help message"
	@echo "=========================================="

# Info target
.PHONY: info
info:
	@echo "=========================================="
	@echo "  Build Configuration"
	@echo "=========================================="
	@echo "Compiler:       $(CC)"
	@echo "CFLAGS:         $(CFLAGS)"
	@echo "LDFLAGS:        $(LDFLAGS)"
	@echo "=========================================="
	@echo "Tasks:"
	@echo "  1. Matrix Multiplication (Block Decomposition)"
	@echo "  2. File Encryption (Chunk-based)"
	@echo "  3. Histogram (Data Partitioning)"
	@echo "  4. Matrix Transpose (Block Partition)"
	@echo "  5. Vector Addition (Element Partitioning)"
	@echo "  6. Sparse Matrix-Vector (CSR Format)"
	@echo "=========================================="

