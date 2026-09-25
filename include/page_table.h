/* ============================================================================
 * page_table.h
 * ----------------------------------------------------------------------------
 * Single-level page table + physical frame manager + page-fault handler.
 *
 * Each virtual page has exactly one page table entry (PTE). The frame
 * manager tracks which physical frames are free/occupied and which virtual
 * page currently owns each frame, which is what makes eviction possible.
 * ==========================================================================*/
#ifndef VMSIM_PAGE_TABLE_H
#define VMSIM_PAGE_TABLE_H

#include <stdint.h>
#include "config.h"
#include "tlb.h"

/* One page-table entry. */
typedef struct {
    int      valid;          /* 1 = page currently resident in a frame     */
    int      frame_number;   /* physical frame holding this page, if valid */
    int      dirty;          /* 1 = page written since being loaded        */
    int      referenced;     /* 1 = page accessed since last clock sweep   */
    uint64_t load_time;      /* logical timestamp used by FIFO             */
    uint64_t last_used;      /* logical timestamp used by LRU              */
} page_table_entry_t;

/* Frame table: reverse mapping frame -> occupying virtual page (-1 = free). */
typedef struct {
    page_table_entry_t entries[NUM_VIRTUAL_PAGES];
    int                frame_owner[NUM_PHYS_FRAMES]; /* virtual page # or -1 */
    unsigned char      physical_memory[PHYS_MEM_SIZE];
    uint64_t           clock;                  /* logical time, incremented
                                                   on every access          */
    replace_policy_t   policy;

    /* running statistics */
    unsigned long page_faults;
    unsigned long page_hits;
    unsigned long evictions;
} page_table_t;

/* Lifecycle */
void page_table_init(page_table_t *pt, replace_policy_t policy);

/*
 * Resolve a virtual page number to a physical frame number.
 * Transparently handles page faults (loading the page from the backing
 * store, evicting a victim page if physical memory is full). If a page is
 * evicted, its stale TLB entry (if any) is invalidated through `tlb`, which
 * keeps the TLB and page table coherent. Pass NULL for tlb to skip this
 * (e.g. in unit tests that don't use a TLB).
 * Returns the physical frame number the page now resides in.
 */
int page_table_resolve(page_table_t *pt, tlb_t *tlb, int page_number,
                        int is_write);

/* Selects a victim frame according to the active replacement policy. */
int page_table_select_victim(page_table_t *pt);

/*
 * Lightweight update used ONLY on the TLB-hit fast path: a real MMU still
 * refreshes the page's referenced/dirty bits and LRU timestamp on every
 * access even when the translation came straight from the TLB, but this
 * must NOT count as a fresh "page table hit" for statistics purposes
 * (that stat is reserved for TLB-miss references that were resolved by
 * consulting the page table). Assumes page_number is already valid.
 */
void page_table_touch(page_table_t *pt, int page_number, int is_write);

#endif /* VMSIM_PAGE_TABLE_H */
