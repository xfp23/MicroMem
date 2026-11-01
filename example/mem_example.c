/**
 * @file example.c
 * @brief MicroMem usage example: table-driven queues allocated at init time.
 *
 * Build on a host machine:
 *   gcc -std=c99 -Wall -Wextra -o example example.c MicroMem.c
 *
 * Flow:
 *   1. MicroMem_Init()
 *   2. Every module allocates its buffers (sizes come from a config table)
 *   3. Check the result, on failure go to a safe state
 *   4. MicroMem_Lock(), no allocation is possible from now on
 *   5. Run
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "MicroMem.h"

/* ------------------------------------------------------------------ */
/* A tiny ring-buffer queue whose storage comes from MicroMem          */
/* ------------------------------------------------------------------ */
typedef struct {
    uint8_t *buf;        /* storage, allocated from MicroMem */
    size_t   item_size;
    size_t   depth;
    size_t   head;
    size_t   tail;
    size_t   count;
} Queue_t;

static bool Queue_Init(Queue_t *q, size_t depth, size_t item_size)
{
    q->buf = (uint8_t *)MicroMem_Alloc(depth, item_size);
    if (q->buf == NULL) {
        return false;
    }
    q->item_size = item_size;
    q->depth     = depth;
    q->head = q->tail = q->count = 0u;
    return true;
}

static bool Queue_Push(Queue_t *q, const void *item)
{
    if (q->count == q->depth) {
        return false;
    }
    memcpy(&q->buf[q->tail * q->item_size], item, q->item_size);
    q->tail = (q->tail + 1u) % q->depth;
    q->count++;
    return true;
}

static bool Queue_Pop(Queue_t *q, void *item)
{
    if (q->count == 0u) {
        return false;
    }
    memcpy(item, &q->buf[q->head * q->item_size], q->item_size);
    q->head = (q->head + 1u) % q->depth;
    q->count--;
    return true;
}

/* ------------------------------------------------------------------ */
/* Configuration: queue depths are decided here, not inside the modules */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
} CanMsg_t;

typedef struct {
    uint16_t event;
    uint16_t param;
} Event_t;

typedef struct {
    const char *name;
    size_t      depth;
    size_t      item_size;
} QueueCfg_t;

enum { Q_CAN_RX, Q_CAN_TX, Q_EVENT, Q_COUNT };

static const QueueCfg_t queue_cfg[Q_COUNT] = {
    [Q_CAN_RX] = { "CAN_RX", 16u, sizeof(CanMsg_t) },
    [Q_CAN_TX] = { "CAN_TX",  8u, sizeof(CanMsg_t) },
    [Q_EVENT]  = { "EVENT",  32u, sizeof(Event_t)  },
};

static Queue_t queues[Q_COUNT];

static void EnterSafeState(const char *reason)
{
    /* In a real system: log a DTC, disable outputs, stay in a safe state */
    printf("[FATAL] %s\n", reason);
    for (;;) { /* never return */ }
}

/* ------------------------------------------------------------------ */
int main(void)
{
    /* 1. Initialize the pool */
    (void)MicroMem_Init();
    printf("Pool free at start : %u bytes\n", (unsigned)MicroMem_Margin());

    /* 2. Allocate every queue from the config table */
    for (size_t i = 0u; i < Q_COUNT; i++) {
        if (!Queue_Init(&queues[i], queue_cfg[i].depth, queue_cfg[i].item_size)) {
            printf("Queue %s does not fit (need %u x %u, free %u)\n",
                   queue_cfg[i].name,
                   (unsigned)queue_cfg[i].depth,
                   (unsigned)queue_cfg[i].item_size,
                   (unsigned)MicroMem_Margin());
            EnterSafeState("MicroMem: configuration does not fit");
        }
        printf("  %-7s depth=%2u item=%2u -> free %u\n",
               queue_cfg[i].name,
               (unsigned)queue_cfg[i].depth,
               (unsigned)queue_cfg[i].item_size,
               (unsigned)MicroMem_Margin());
    }

    /* Remaining bytes tell you how far MICROMEM_BUFFER_SIZE can be reduced */
    printf("Pool free after init: %u bytes\n", (unsigned)MicroMem_Margin());

    /* 3. Lock: no allocation at run time */
    MicroMem_Lock();

    if (MicroMem_Alloc(1u, 1u) == NULL) {
        printf("Alloc after lock   : refused (as expected)\n");
    }

    /* 4. Run: use the queues */
    CanMsg_t tx = { .id = 0x123u, .dlc = 3u, .data = { 0xAAu, 0xBBu, 0xCCu } };
    CanMsg_t rx;

    (void)Queue_Push(&queues[Q_CAN_RX], &tx);
    if (Queue_Pop(&queues[Q_CAN_RX], &rx)) {
        printf("CAN_RX popped      : id=0x%03X dlc=%u data[0]=0x%02X\n",
               (unsigned)rx.id, (unsigned)rx.dlc, (unsigned)rx.data[0]);
    }

    Event_t ev = { .event = 1u, .param = 42u };
    Event_t ev_out;
    (void)Queue_Push(&queues[Q_EVENT], &ev);
    if (Queue_Pop(&queues[Q_EVENT], &ev_out)) {
        printf("EVENT popped       : event=%u param=%u\n",
               (unsigned)ev_out.event, (unsigned)ev_out.param);
    }

    return 0;
}