/* ============================================================================
 * page_table.c  -  see page_table.h for module contract
 * ==========================================================================*/
#include <string.h>
#include <stdio.h>
#include "../include/page_table.h"
#include "../include/backing_store.h"

void page_table_init(page_table_t *pt, replace_policy_t policy)
{
    memset(pt, 0, sizeof(*pt));
    for (int i = 0; i < NUM_VIRTUAL_PAGES; i++) {
        pt->entries[i].valid = 0;
    }
    for (int i = 0; i < NUM_PHYS_FRAMES; i++) {
        pt->frame_owner[i] = -1;
    }
    pt->clock  = 0;
    pt->policy = policy;
    pt->page_faults = 0;
    pt->page_hits   = 0;
    pt->evictions   = 0;
}

/* Returns the index of the first free physical frame, or -1 if none free. */
static int find_free_frame(page_table_t *pt)
{
    for (int i = 0; i < NUM_PHYS_FRAMES; i++) {
        if (pt->frame_owner[i] == -1) {
            return i;
        }
    }
    return -1;
}

int page_table_select_victim(page_table_t *pt)
{
    int victim_frame = 0;

    if (pt->policy == POLICY_FIFO) {
        /* Victim = the resident page with the smallest load_time. */
        uint64_t oldest = UINT64_MAX;
        for (int f = 0; f < NUM_PHYS_FRAMES; f++) {
            int vp = pt->frame_owner[f];
            if (vp != -1 && pt->entries[vp].load_time < oldest) {
                oldest = pt->entries[vp].load_time;
                victim_frame = f;
            }
        }
    } else { /* POLICY_LRU */
        /* Victim = the resident page with the smallest last_used time. */
        uint64_t oldest = UINT64_MAX;
        for (int f = 0; f < NUM_PHYS_FRAMES; f++) {
            int vp = pt->frame_owner[f];
            if (vp != -1 && pt->entries[vp].last_used < oldest) {
                oldest = pt->entries[vp].last_used;
                victim_frame = f;
            }
        }
    }
    return victim_frame;
}

void page_table_touch(page_table_t *pt, int page_number, int is_write)
{
    pt->clock++;
    page_table_entry_t *pte = &pt->entries[page_number];
    pte->referenced = 1;
    pte->last_used  = pt->clock;
    if (is_write) pte->dirty = 1;
}

int page_table_resolve(page_table_t *pt, tlb_t *tlb, int page_number,
                        int is_write)
{
    pt->clock++;
    page_table_entry_t *pte = &pt->entries[page_number];

    if (pte->valid) {
        /* ---- Page hit: page already resident in a physical frame ---- */
        pt->page_hits++;
        pte->referenced = 1;
        pte->last_used  = pt->clock;
        if (is_write) pte->dirty = 1;
        return pte->frame_number;
    }

    /* ---------------------- Page fault path --------------------------- */
    pt->page_faults++;

    int frame = find_free_frame(pt);
    if (frame == -1) {
        /* Physical memory full -> evict a victim page. */
        frame = page_table_select_victim(pt);
        int victim_page = pt->frame_owner[frame];

        /* In a real OS, a dirty victim would be written back to disk here.
         * We only need to note the fact for statistics/reporting. */
        page_table_entry_t *victim_pte = &pt->entries[victim_page];
        victim_pte->valid = 0;
        pt->evictions++;

        if (tlb != NULL) {
            tlb_invalidate(tlb, victim_page); /* keep TLB coherent */
        }
    }

    /* Load the faulting page from the backing store into the chosen frame. */
    backing_store_read_page(page_number,
                             &pt->physical_memory[frame * PAGE_SIZE]);

    pt->frame_owner[frame] = page_number;
    pte->valid       = 1;
    pte->frame_number = frame;
    pte->dirty        = is_write ? 1 : 0;
    pte->referenced    = 1;
    pte->load_time     = pt->clock;
    pte->last_used      = pt->clock;

    return frame;
}
