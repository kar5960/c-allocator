# c-allocator

A low-level C memory allocator built as an educational project to explore custom heap management, virtual memory mechanics, and kernel system call interfaces. It enforces 16-byte alignment, using `sbrk` for small heap 
chunks and `mmap` for allocations over 128KB.

## Purpose & Learnings
This project was created from scratch to understand the internal mechanics of dynamic memory allocation:
- **System Call Mechanics**: Interacting directly with the Linux kernel via `sbrk`/`brk` heap expansion and `mmap`/`munmap` page allocations.
- **Header & Free-List Logic**: Managing doubly-linked free lists, split-block algorithms, and adjacent block coalescing.
- **Memory Alignment & Headers**: Enforcing strict 16-byte alignment across `block_header_t` layouts and payload boundaries.
- **Process Isolation**: Constructing a C test harness using `fork()` to execute each test case in an isolated child process, preventing state contamination between runs.

## Project Layout

```text
.
├── include/     # Header files (allocator.h, internal.h)
├── src/         # Core logic (core.c, strategy.c)
├── tests/       # Test runner (test_main.c)
└── Makefile     # Build automation and diagnostic tooling
```

## How to Use the Makefile

The project includes an automated `Makefile` to simplify compilation, testing, and system-level diagnostics.

### 1. Build the Binary
To compile the project without running tests:
```bash
make
```
*Creates an `obj/` directory, compiles all source files into object files, and links the `test_runner` executable.*

### 2. Run the Test Harness
To compile and execute all isolated unit tests:
```bash
make test
```

### 3. Diagnostics & System Tracing
Run memory safety and system call checks using built-in tooling targets:

* **Valgrind (Memory Inspection):**
  ```bash
  make valgrind
  ```
  *Executes the test suite under Valgrind to check for out-of-bounds reads/writes and memory corruption.*

* **strace (Syscall Tracing):**
  ```bash
  make strace
  ```
  *Intercepts and logs real-time `brk`, `mmap`, and `munmap` system calls made to the Linux kernel.*

### 4. Cleanup
To remove all generated object files and executable binaries:
```bash
make clean
```
