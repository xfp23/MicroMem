/**
 * @file MicroMem.c
 * @author xfp23
 * @brief
 * @version 0.1
 * @date 2026-11-01
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "MicroMem.h"

static MicroMem_Obj_t mem_obj;

MicroMem_Status_t MicroMem_Init(void)
{
    memset(&mem_obj, 0, sizeof(mem_obj));
    return MICROMEM_STA_OK;
}

void *MicroMem_Alloc(size_t num, size_t size)
{
    if (mem_obj.lock || num == 0u || size == 0u)
    {
        return NULL;
    }
    if (num > SIZE_MAX / size)
    {
        return NULL;
    }

    size_t raw = num * size;
    if (raw > SIZE_MAX - (MICROMEM_ALIGN - 1u))
    {
        return NULL;
    }

    size_t aligned = MICROMEM_ALIGN_UP(raw);
    if (aligned > (size_t)MICROMEM_BUFFER_SIZE - mem_obj.used)
    {
        return NULL; /* 容量不足 */
    }

    void *p = &mem_obj.ram[mem_obj.used];
    mem_obj.used += aligned;
    return p;
}

size_t MicroMem_Margin(void)
{
    return (size_t)MICROMEM_BUFFER_SIZE - mem_obj.used;
}

void MicroMem_Lock(void)
{
    mem_obj.lock = true;
}

// MicroMem_Status_t MicroMem_Reset(void)
// {
//     if (mem_obj.lock)
//     {
//         return MICROMEM_STA_ERR;
//     }

//     memset(mem_obj.ram, 0, sizeof(mem_obj.ram));
//     mem_obj.used = 0u;

//     return MICROMEM_STA_OK;
// }