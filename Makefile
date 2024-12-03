# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g -O4
LDLIBS = -lm -lrt

# Directories
SRC_DIR = src
BUILD_DIR = build
INSTALL_DIR = bin

# Targets
SEQ_TARGET = stencil_seq
MPI_TARGET = stencil_mpi
OMP_TARGET = stencil_omp
HYBRID_TARGET = stencil_hybrid
TARGETS = $(SEQ_TARGET) $(MPI_TARGET) $(OMP_TARGET) $(HYBRID_TARGET)

# Source Files
SEQ_SRC = $(SRC_DIR)/stencil_seq.c
MPI_SRC = $(SRC_DIR)/stencil_mpi.c
OMP_SRC = $(SRC_DIR)/stencil_omp.c
HYBRID_SRC = $(SRC_DIR)/stencil_hybrid.c

# Object Files
SEQ_OBJ = $(BUILD_DIR)/stencil_seq.o
MPI_OBJ = $(BUILD_DIR)/stencil_mpi.o
OMP_OBJ = $(BUILD_DIR)/stencil_omp.o
HYBRID_OBJ = $(BUILD_DIR)/stencil_hybrid.o

# MPI and OpenMP Specific Flags
MPI_LDLIBS = -lmpi
OMP_CFLAGS = -fopenmp

# Default Rule
all: $(TARGETS)

# Sequential Target
$(SEQ_TARGET): $(SEQ_OBJ) | $(INSTALL_DIR)
	$(CC) $(CFLAGS) -o $(INSTALL_DIR)/$@ $< $(LDLIBS)

# MPI Target
$(MPI_TARGET): $(MPI_OBJ) | $(INSTALL_DIR)
	$(CC) $(CFLAGS) -o $(INSTALL_DIR)/$@ $< $(LDLIBS) $(MPI_LDLIBS)

# OpenMP Target
$(OMP_TARGET): $(OMP_OBJ) | $(INSTALL_DIR)
	$(CC) $(CFLAGS) $(OMP_CFLAGS) -o $(INSTALL_DIR)/$@ $< $(LDLIBS)

# Hybrid (MPI + OpenMP) Target
$(HYBRID_TARGET): $(HYBRID_OBJ) | $(INSTALL_DIR)
	$(CC) $(CFLAGS) $(OMP_CFLAGS) -o $(INSTALL_DIR)/$@ $< $(LDLIBS) $(MPI_LDLIBS)

# Compile Object Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories if not present
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(INSTALL_DIR):
	@mkdir -p $(INSTALL_DIR)

# Clean Rule
clean:
	-rm -rf $(BUILD_DIR)/* $(INSTALL_DIR)/*

.PHONY: all clean

