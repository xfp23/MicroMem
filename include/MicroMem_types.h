/* MicroMem_types.h */
#ifndef MICROMEM_TYPES_H
#define MICROMEM_TYPES_H

#include "MicroMem_conf.h"
#include "MicroMem_utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char MicroMem_size_check_t[(MICROMEM_BUFFER_SIZE % MICROMEM_ALIGN == 0u) ? 1 : -1];

typedef enum {
    MICROMEM_STA_OK = 0,
    MICROMEM_STA_ERR,
} MicroMem_Status_t;

typedef struct {
    uint8_t ram[MICROMEM_BUFFER_SIZE] __attribute__((aligned(MICROMEM_ALIGN)));
    size_t  used;   /* 已使用（含對齊填充） */
    bool    lock;
} MicroMem_Obj_t;

#ifdef __cplusplus
}
#endif
#endif