/* ============================================================================
 * tlb.h
 * ----------------------------------------------------------------------------
 * Fully-associative Translation Lookaside Buffer (software model).
 *
 * A real TLB is content-addressable hardware; here it is modelled as a small
 * array searched linearly (TLB_SIZE is tiny, so this is not a performance
 * problem and keeps the code readable). Replacement is FIFO, which is what
 * most real TLBs use because recency bits are expensive in hardware.
 * ==========================================================================*/
#ifndef VMSIM_TLB_H
#define VMSIM_TLB_H

#include <stdint.h>
#include "config.h"

typedef struct {
    int      valid;
    int      page_number;
    int      frame_number;
    uint64_t load_time;    /* logical timestamp -> used for FIFO eviction */
} tlb_entry_t;

typedef struct {
    tlb_entry_t entries[TLB_SIZE];
    int         next_victim;   /* FIFO circular pointer */
    uint64_t    clock;

    /* running statistics */
    unsigned long hits;
    unsigned long misses;
} tlb_t;

void tlb_init(tlb_t *tlb);

/* Returns the physical frame number on a hit, or -1 on a miss. */
int  tlb_lookup(tlb_t *tlb, int page_number);

/* Inserts/updates a translation (called after a page table lookup). */
void tlb_insert(tlb_t *tlb, int page_number, int frame_number);

/* Must be called whenever a page is evicted from the page table, so any
 * stale TLB entry pointing at it is invalidated (TLB/page-table coherence). */
void tlb_invalidate(tlb_t *tlb, int page_number);

#endif /* VMSIM_TLB_H */
