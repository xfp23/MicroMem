# MicroMem

[English](./readme.md) | [简体中文](./readme.zh.md)

A tiny, deterministic static memory pool for embedded and automotive projects.
Memory is allocated **only during initialization** and is **never freed
individually**, so allocation time is constant and there is no fragmentation.

## Why

Libraries such as an RTOS, a message scheduler or a queue module often need
variable-sized buffers. Static arrays sized for the worst case waste RAM, while
`malloc` / `calloc` / `free` are not acceptable in automotive projects because
their execution time is not bounded.

MicroMem takes a middle path:

- One statically allocated pool (lives in `.bss`).
- Modules request their memory once at init time, with sizes taken from a
  configuration instead of being hard-coded.
- The pool is then locked. No allocation happens at run time.

## Features

- O(1) allocation, no search, no fragmentation
- No dynamic free, behavior is fully deterministic
- calloc-style API, returned memory is zero-initialized
- Configurable alignment (default 8 bytes)
- Overflow-safe size calculation
- One-way lock to forbid run-time allocation
- Pure C99, no dependencies besides the standard headers, GCC compatible

## Files

| File | Description |
|------|-------------|
| `MicroMem.h` | Public API |
| `MicroMem.c` | Implementation |
| `MicroMem_types.h` | Types and the pool object |
| `MicroMem_conf.h` | Configuration (version, alignment, pool size) |
| `MicroMem_utils.h` | Helper macros |

## Configuration

Edit `MicroMem_conf.h`:

| Macro | Default | Description |
|-------|---------|-------------|
| `MICROMEM_VERSION` | `"0.0.1"` | Library version |
| `MICROMEM_ALIGN` | `8u` | Alignment in bytes, must be a power of two |
| `MICROMEM_BUFFER_SIZE` | `1024u` | Pool size in bytes, must be a multiple of `MICROMEM_ALIGN` (checked at compile time) |

## API

| Function | Description |
|----------|-------------|
| `MicroMem_Status_t MicroMem_Init(void)` | Clear the pool and the lock flag. Call once at startup. |
| `void *MicroMem_Alloc(size_t num, size_t size)` | Allocate `num * size` bytes, zeroed and aligned. Returns `NULL` on failure. |
| `size_t MicroMem_Margin(void)` | Remaining capacity in bytes. |
| `void MicroMem_Lock(void)` | Forbid further allocation. One-way. |

`MicroMem_Alloc` returns `NULL` when:

- the pool is locked
- `num` or `size` is zero
- `num * size` (or the aligned size) overflows
- the remaining capacity is not enough

## Usage

```c
#include "MicroMem.h"

typedef struct {
    uint32_t id;
    uint8_t  data[8];
} Msg_t;

int main(void)
{
    MicroMem_Init();

    /* Each module allocates what it needs at init time */
    Msg_t *rx_queue = MicroMem_Alloc(16, sizeof(Msg_t));
    Msg_t *tx_queue = MicroMem_Alloc(8,  sizeof(Msg_t));

    if (rx_queue == NULL || tx_queue == NULL) {
        /* Configuration does not fit: enter a safe state / report an error */
    }

    /* Optional: check how much is left, useful for sizing the pool */
    size_t left = MicroMem_Margin();
    (void)left;

    /* No allocation is allowed from now on */
    MicroMem_Lock();

    /* ... run ... */
    return 0;
}
```

## Behavior notes

- **Alignment**: the pool start and every returned address are aligned to
  `MICROMEM_ALIGN`. Each block size is rounded up to a multiple of it, so the
  real consumption can be slightly larger than requested.
- **Zero-initialized**: `Init` clears the pool and blocks are never reused, so
  every returned block is all zeros.
- **Init**: also clears the lock flag. It invalidates all previous pointers.
- **No Reset API**: the former `MicroMem_Reset()` has been removed. Since the
  pool is allocated once at init time and then locked, there is nothing to
  reclaim at run time. To start over, call `MicroMem_Init()`.
- **No unlock**: this is intentional.

## Limitations

- Not thread-safe. Allocate from a single context during startup, or protect
  the calls with your own critical section.
- No individual free. If you need run-time allocation, use a separate
  fixed-size block pool.
- No out-of-bounds protection. A buffer overrun corrupts the neighboring block.

## Roadmap ideas

- Failure hook and detailed error codes
- Named allocations and high-water mark statistics (debug builds)
- Aligned allocation with custom alignment (DMA, cache line)
- Placement into a specific RAM section, multiple pools
- Guard words for overflow detection
- Mark / rollback during initialization

## Version

`0.0.1`

## License

[LICENSE](./LICENSE)