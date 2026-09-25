/* ============================================================================
 * tlb.c  -  see tlb.h for module contract
 * ==========================================================================*/
#include <string.h>
#include "../include/tlb.h"

void tlb_init(tlb_t *tlb)
{
    memset(tlb, 0, sizeof(*tlb));
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb->entries[i].valid = 0;
    }
    tlb->next_victim = 0;
    tlb->clock  = 0;
    tlb->hits   = 0;
    tlb->misses = 0;
}

int tlb_lookup(tlb_t *tlb, int page_number)
{
    tlb->clock++;
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].valid &&
            tlb->entries[i].page_number == page_number) {
            tlb->hits++;
            return tlb->entries[i].frame_number;
        }
    }
    tlb->misses++;
    return -1;
}

void tlb_insert(tlb_t *tlb, int page_number, int frame_number)
{
    /* If already present, just refresh the frame number (handles the rare
     * case where the same page is reloaded into a different frame). */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].valid &&
            tlb->entries[i].page_number == page_number) {
            tlb->entries[i].frame_number = frame_number;
            return;
        }
    }

    /* Prefer an empty slot before evicting anything (FIFO fill-then-evict). */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!tlb->entries[i].valid) {
            tlb->entries[i].valid        = 1;
            tlb->entries[i].page_number  = page_number;
            tlb->entries[i].frame_number = frame_number;
            tlb->entries[i].load_time    = tlb->clock;
            return;
        }
    }

    /* TLB full: evict via circular FIFO pointer. */
    int victim = tlb->next_victim;
    tlb->entries[victim].valid        = 1;
    tlb->entries[victim].page_number  = page_number;
    tlb->entries[victim].frame_number = frame_number;
    tlb->entries[victim].load_time    = tlb->clock;
    tlb->next_victim = (tlb->next_victim + 1) % TLB_SIZE;
}

void tlb_invalidate(tlb_t *tlb, int page_number)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].valid &&
            tlb->entries[i].page_number == page_number) {
            tlb->entries[i].valid = 0;
        }
    }
}
