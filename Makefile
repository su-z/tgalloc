# Makefile for tgalloc library and tests

# Compiler and flags
CC = clang
CFLAGS = -std=c2x -Wall -Wextra -pedantic -O2
CPPFLAGS = -I.

# Directories
BUILD_DIR = build
TEST_DIR = tests
TEST_BUILD_DIR = $(BUILD_DIR)/tests

# Source files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%,$(TEST_SRCS))

# Phony targets
.PHONY: all clean test dirs

# Default target
all: test

# Create required directories
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(TEST_BUILD_DIR)

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Build and run all tests
test: $(TEST_BINS)
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo "\n=== Running $$test ==="; \
		$$test; \
		if [ $$? -ne 0 ]; then \
			echo "Test $$test failed!"; \
			exit 1; \
		fi; \
	done
	@echo "\nAll tests passed!"

# Compile test object files
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c | dirs
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Link test executables
$(TEST_BUILD_DIR)/%: $(TEST_BUILD_DIR)/%.o | dirs
	$(CC) $(CFLAGS) $< -o $@

# Individual test targets
tgalloc_tests: $(TEST_BUILD_DIR)/tgalloc_tests
	$(TEST_BUILD_DIR)/tgalloc_tests

allocator_switching: $(TEST_BUILD_DIR)/test_allocator_switching
	$(TEST_BUILD_DIR)/test_allocator_switching

# Generate test dependencies
$(TEST_BUILD_DIR)/tgalloc_tests: $(TEST_DIR)/tgalloc_tests.c tgalloc.h | dirs
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

$(TEST_BUILD_DIR)/test_allocator_switching: $(TEST_DIR)/test_allocator_switching.c tgalloc.h tgalloc_default.h | dirs
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

# Static analysis
.PHONY: analyze
analyze: dirs
	@echo "Running static analysis..."
	@for src in $(TEST_SRCS); do \
		echo "Analyzing $$src..."; \
		clang --analyze $(CPPFLAGS) $(CFLAGS) $$src -o $(BUILD_DIR)/dummy.o || exit 1; \
	done
	@echo "Static analysis completed successfully."
	@rm -f $(BUILD_DIR)/dummy.o

# Header-only library verification (for C11 compatibility)
.PHONY: verify
verify: dirs
	@echo "Verifying header-only library..."
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only tgalloc.h
	@echo "Header verification completed successfully."

# Documentation (requires Doxygen)
.PHONY: docs
docs:
	@command -v doxygen >/dev/null 2>&1 || { echo "Doxygen not installed. Skipping documentation generation."; exit 0; }
	@echo "Generating documentation..."
	@mkdir -p docs
	@doxygen Doxyfile || echo "Documentation generation failed. Continuing anyway."
	@echo "Documentation generated in docs/ directory."

# Install target for header-only library
.PHONY: install
install:
	@echo "Installing header files to /usr/local/include..."
	@mkdir -p /usr/local/include/tgalloc
	@cp tgalloc.h tgalloc_default.h /usr/local/include/tgalloc/
	@echo "Installation complete."

# Help target
.PHONY: help
help:
	@echo "tgalloc Makefile targets:"
	@echo "  all			- Build and run all tests (default)"
	@echo "  test		   - Build and run all tests"
	@echo "  tgalloc_tests  - Run the main tgalloc tests"
	@echo "  allocator_switching - Run the allocator switching tests"
	@echo "  clean		  - Remove build artifacts"
	@echo "  analyze		- Run static analysis on the code"
	@echo "  verify		 - Verify the header-only library compiles"
	@echo "  docs		   - Generate documentation (requires Doxygen)"
	@echo "  install		- Install headers to /usr/local/include"
	@echo "  help		   - Display this help message"