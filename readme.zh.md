# MicroMem

[English](./readme.md) | [简体中文](./readme.zh.md)

一个面向嵌入式与车规项目的微型确定性静态内存池。
内存**只在初始化阶段分配**，且**不支持单独释放**，因此分配耗时恒定，也不会产生碎片。

## 为什么需要它

RTOS、报文调度器、队列等库经常需要可变长度的缓冲区。按最坏情况用静态数组会浪费 RAM，
而 `malloc` / `calloc` / `free` 的执行时间不可控，在车规项目中是不允许的。

MicroMem 走的是折中路线：

- 一块静态分配的内存池（位于 `.bss`）。
- 各模块在初始化时一次性申请内存，大小由配置决定，而不是写死在代码里。
- 之后锁定内存池，运行期不再有任何分配动作。

## 特性

- O(1) 分配，无查找、无碎片
- 无动态释放，行为完全确定
- calloc 风格接口，返回的内存已清零
- 可配置对齐（默认 8 字节）
- 防溢出的大小计算
- 单向锁定，禁止运行期分配
- 纯 C99，仅依赖标准头文件，兼容 GCC

## 文件说明

| 文件 | 说明 |
|------|------|
| `MicroMem.h` | 对外接口 |
| `MicroMem.c` | 实现 |
| `MicroMem_types.h` | 类型与内存池对象 |
| `MicroMem_conf.h` | 配置（版本、对齐、池大小） |
| `MicroMem_utils.h` | 辅助宏 |

## 配置

修改 `MicroMem_conf.h`：

| 宏 | 默认值 | 说明 |
|----|--------|------|
| `MICROMEM_VERSION` | `"0.0.1"` | 库版本 |
| `MICROMEM_ALIGN` | `8u` | 对齐字节数，必须是 2 的幂 |
| `MICROMEM_BUFFER_SIZE` | `1024u` | 内存池大小（字节），必须是 `MICROMEM_ALIGN` 的整数倍（编译期检查） |

## 接口

| 函数 | 说明 |
|------|------|
| `MicroMem_Status_t MicroMem_Init(void)` | 清空内存池与锁定标志，启动时调用一次。 |
| `void *MicroMem_Alloc(size_t num, size_t size)` | 申请 `num * size` 字节，已清零且已对齐，失败返回 `NULL`。 |
| `size_t MicroMem_Margin(void)` | 剩余容量（字节）。 |
| `void MicroMem_Lock(void)` | 禁止后续分配，单向不可撤销。 |

`MicroMem_Alloc` 在以下情况返回 `NULL`：

- 内存池已锁定
- `num` 或 `size` 为 0
- `num * size`（或对齐后的大小）发生溢出
- 剩余容量不足

## 使用示例

```c
#include "MicroMem.h"

typedef struct {
    uint32_t id;
    uint8_t  data[8];
} Msg_t;

int main(void)
{
    MicroMem_Init();

    /* 各模块在初始化阶段申请所需内存 */
    Msg_t *rx_queue = MicroMem_Alloc(16, sizeof(Msg_t));
    Msg_t *tx_queue = MicroMem_Alloc(8,  sizeof(Msg_t));

    if (rx_queue == NULL || tx_queue == NULL) {
        /* 配置装不下：进入安全状态 / 上报错误 */
    }

    /* 可选：查看剩余量，用于确定池的大小 */
    size_t left = MicroMem_Margin();
    (void)left;

    /* 此后不再允许分配 */
    MicroMem_Lock();

    /* ... 运行 ... */
    return 0;
}
```

## 行为说明

- **对齐**：池的起始地址和每次返回的地址都按 `MICROMEM_ALIGN` 对齐。
  每个块的大小会向上取整到对齐值的整数倍，实际占用可能略大于申请量。
- **零初始化**：`Init` 会清零内存池，且内存块不会被复用，所以返回的块一定全为 0。
- **Init**：同样会清除锁定标志，并使之前所有指针失效。
- **没有 Reset 接口**：原来的 `MicroMem_Reset()` 已移除。内存池只在初始化阶段分配，之后锁定，
  运行期没有需要回收的内容。需要重来时调用 `MicroMem_Init()`。
- **没有 Unlock**：这是有意为之。

## 限制

- 非线程安全。请在启动阶段的单一上下文中分配，或自行加临界区保护。
- 不支持单独释放。如果需要运行期分配，请另用固定大小块的内存池。
- 没有越界保护。缓冲区越界会破坏相邻的内存块。

## 后续扩展方向

- 分配失败钩子与详细错误码
- 带名称的分配与高水位统计（调试版本）
- 自定义对齐的分配（DMA、Cache line）
- 指定放入特定 RAM 段、多内存池
- 用于检测越界的保护字
- 初始化阶段的标记 / 回滚

## 版本

`0.0.1`

## 许可证

[许可证](./LICENSE)