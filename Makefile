# Compiler and Flags
CC_SEQ     = gcc
CC_MPI     = mpicc
CFLAGS_SEQ = -Wall -g -O0
CFLAGS_MPI = -Wall -g -O0 -fopenmp
LDLIBS_SEQ = -lm -lrt
LDLIBS_MPI = -lm -lrt -lmpi

# Directories
SRC_DIR = src
BUILD_DIR = build
INSTALL_DIR = bin

# Targets
TARGETS = stencil_seq stencil_mpi stencil_omp stencil_hybrid

.PRECIOUS: $(BUILD_DIR)/%.o

# Rules
all: $(addprefix $(INSTALL_DIR)/, $(TARGETS))

# Build Rule for Sequential Target
$(INSTALL_DIR)/stencil_seq: $(BUILD_DIR)/stencil_seq.o | $(INSTALL_DIR)
	$(CC_SEQ) $(CFLAGS_SEQ) -o $@ $< $(LDLIBS_SEQ)

# Build Rule for MPI and Hybrid Targets
$(INSTALL_DIR)/%: $(BUILD_DIR)/%.o | $(INSTALL_DIR)
	$(CC_MPI) $(CFLAGS_MPI) -o $@ $< $(LDLIBS_MPI)

# General Rule for Object Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
ifeq ($(@F),stencil_seq.o)
	$(CC_SEQ) $(CFLAGS_SEQ) -c $< -o $@
else
	$(CC_MPI) $(CFLAGS_MPI) -c $< -o $@
endif

# Directory Creation
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(INSTALL_DIR):
	@mkdir -p $(INSTALL_DIR)

# Clean Rule
clean:
	-rm -rf $(BUILD_DIR) $(INSTALL_DIR)

.PHONY: all clean

