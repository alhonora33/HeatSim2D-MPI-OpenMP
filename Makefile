# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g -O0 -fopenmp
LDLIBS = -lm -lrt -lmpi

# Directories
SRC_DIR = src
BUILD_DIR = build
INSTALL_DIR = bin

# Targets
TARGETS = stencil_seq stencil_mpi stencil_omp stencil_hybrid

.PRECIOUS: $(BUILD_DIR)/%.o

# Rules
all: $(addprefix $(INSTALL_DIR)/, $(TARGETS))

# General Build Rule for Executables
$(INSTALL_DIR)/%: $(BUILD_DIR)/%.o | $(INSTALL_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

# General Rule for Object Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Directory Creation
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(INSTALL_DIR):
	@mkdir -p $(INSTALL_DIR)

# Clean Rule
clean:
	-rm -rf $(BUILD_DIR) $(INSTALL_DIR)

.PHONY: all clean

