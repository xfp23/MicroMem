#ifndef MICROMEM_UTILS_H
#define MICROMEM_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MICROMEM_ALIGN_UP(x) (((x) + (MICROMEM_ALIGN - 1u)) & ~(size_t)(MICROMEM_ALIGN - 1u))

#ifdef __cplusplus
}
#endif

#endif
