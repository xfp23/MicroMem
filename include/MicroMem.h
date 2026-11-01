/**
 * @file MicroMem.h
 * @author https://xfp23.github.io/
 * @brief Deterministic, init-time-only static memory pool (no free, no fragmentation).
 * @version \ref MICROMEM_VERSION
 * @date 2026-10-06
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef MICROMEM_H
#define MICROMEM_H

#include "MicroMem_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Initialize the memory pool.
 *
 * Clears the whole pool (including the lock flag) and resets the allocation
 * cursor. Must be called once before any other MicroMem API.
 *
 * @note All pointers returned before this call become invalid.
 * @note Not thread-safe. Call it from a single context during startup.
 *
 * @return MICROMEM_STA_OK  Always, in the current implementation.
 */
extern MicroMem_Status_t MicroMem_Init(void);

/**
 * @brief Allocate a zero-initialized block from the pool (calloc-style).
 *
 * The block size is rounded up to a multiple of @ref MICROMEM_ALIGN and the
 * returned address is aligned to @ref MICROMEM_ALIGN. Allocation is O(1),
 * there is no search and no fragmentation. Memory cannot be freed
 * individually; call MicroMem_Init() to start over.
 *
 * Intended to be called during initialization only, then followed by
 * MicroMem_Lock().
 *
 * @param _NumOfElements   Number of elements. Must be non-zero.
 * @param _SizeOfElements  Size of one element in bytes. Must be non-zero.
 *
 * @return Pointer to the zeroed block on success.
 * @retval NULL  The pool is locked, an argument is zero, the size calculation
 *               overflows, or the remaining capacity is insufficient.
 *
 * @note The caller MUST check the return value.
 * @note Not thread-safe.
 */
extern void *MicroMem_Alloc(size_t _NumOfElements, size_t _SizeOfElements);

/**
 * @brief Get the remaining capacity of the pool.
 *
 * Useful at startup to verify that the configuration fits, and to size
 * MICROMEM_BUFFER_SIZE (remaining bytes after all modules are initialized).
 *
 * @return Remaining bytes available for allocation.
 */
extern size_t MicroMem_Margin(void);

/**
 * @brief Lock the pool.
 *
 * After locking, MicroMem_Alloc() returns NULL. The lock is one-way:
 * there is no unlock. Only MicroMem_Init() clears it.
 *
 * Call this once all modules have finished their init-time allocations, so
 * that no allocation can happen at run time.
 */
extern void MicroMem_Lock(void);

#ifdef __cplusplus
}
#endif

#endif