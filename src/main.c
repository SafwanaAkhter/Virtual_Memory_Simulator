/* ============================================================================
 * main.c
 * ----------------------------------------------------------------------------
 * Driver program for the Virtual Memory Simulator.
 *
 * Pipeline for every logical (virtual) memory reference:
 *
 *      virtual address
 *            |
 *            v
 *    split into (page number, offset)
 *            |
 *            v
 *        TLB lookup ----------- hit ------> physical frame
 *            |
 *          miss
 *            |
 *            v
 *    page table lookup ---- resident ----> physical frame  (page hit)
 *            |                                   |
 *       not resident (PAGE FAULT)                |
 *            |                                   |
 *            v                                   v
 *    load page from backing store,        insert (page,frame)
 *    evict a victim page if memory full        into TLB
 *            |                                   |
 *            +---------------->-------------------+
 *                              |
 *                              v
 *                 physical address = frame*PAGE_SIZE + offset
 *
 * Usage:
 *   ./vmsim <address_file> [fifo|lru] [-v]
 *
 *   <address_file>  text file, one decimal virtual address per line
 *   fifo|lru        page replacement policy (default: lru)
 *   -v              verbose: print the translation of every reference
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"
#include "../include/tlb.h"
#include "../include/page_table.h"
#include "../include/backing_store.h"

/* Splits a 16-bit virtual address into page number and offset. */
static void split_address(int vaddr, int *page_number, int *offset)
{
    *offset      = vaddr & (PAGE_SIZE - 1);
    *page_number = (vaddr >> PAGE_OFFSET_BITS) & (NUM_VIRTUAL_PAGES - 1);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <address_file> [fifo|lru] [-v]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *address_file = argv[1];
    replace_policy_t policy = POLICY_LRU;
    int verbose = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "fifo") == 0) policy = POLICY_FIFO;
        else if (strcmp(argv[i], "lru") == 0) policy = POLICY_LRU;
        else if (strcmp(argv[i], "-v") == 0) verbose = 1;
    }

    FILE *addr_fp = fopen(address_file, "r");
    if (addr_fp == NULL) {
        fprintf(stderr, "FATAL: cannot open address file '%s'\n", address_file);
        return EXIT_FAILURE;
    }

    backing_store_ensure_exists();

    tlb_t tlb;
    page_table_t pt;
    tlb_init(&tlb);
    page_table_init(&pt, policy);

    printf("=========================================================\n");
    printf(" Virtual Memory Simulator\n");
    printf(" Page size            : %d bytes\n", PAGE_SIZE);
    printf(" Virtual address space: %d pages (%d bytes)\n",
           NUM_VIRTUAL_PAGES, NUM_VIRTUAL_PAGES * PAGE_SIZE);
    printf(" Physical memory      : %d frames (%d bytes)\n",
           NUM_PHYS_FRAMES, PHYS_MEM_SIZE);
    printf(" TLB size             : %d entries (fully-associative, FIFO)\n",
           TLB_SIZE);
    printf(" Page replacement     : %s\n",
           policy == POLICY_FIFO ? "FIFO" : "LRU");
    printf("=========================================================\n\n");

    if (verbose) {
        printf("%-8s %-6s %-8s %-6s %-6s %-8s %-6s\n",
               "VAddr", "Page", "Offset", "TLB", "PTbl", "Frame", "Value");
        printf("-----------------------------------------------------------\n");
    }

    long total_refs = 0;
    char line[64];
    while (fgets(line, sizeof(line), addr_fp) != NULL) {
        if (line[0] == '\n' || line[0] == '#') continue; /* skip blanks/comments */
        int vaddr = atoi(line);
        if (vaddr < 0 || vaddr >= (NUM_VIRTUAL_PAGES * PAGE_SIZE)) {
            fprintf(stderr, "WARNING: address %d out of range, skipped\n", vaddr);
            continue;
        }

        int page_number, offset;
        split_address(vaddr, &page_number, &offset);
        total_refs++;

        const char *tlb_status;
        const char *pt_status;

        int frame = tlb_lookup(&tlb, page_number);
        if (frame != -1) {
            tlb_status = "HIT";
            pt_status  = "-";
            /* Still touch the PTE so LRU/reference bits stay accurate even
             * on a TLB hit -- a real MMU updates the accessed bit in DRAM
             * on every reference, TLB hit or not. This does NOT count as a
             * page-table statistics event (see page_table_touch doc). */
            page_table_touch(&pt, page_number, 0);
        } else {
            tlb_status = "MISS";
            unsigned long faults_before = pt.page_faults;
            frame = page_table_resolve(&pt, &tlb, page_number, 0);
            pt_status = (pt.page_faults != faults_before) ? "FAULT" : "HIT";
            tlb_insert(&tlb, page_number, frame);
        }

        int physical_addr = frame * PAGE_SIZE + offset;
        unsigned char value = pt.physical_memory[physical_addr];

        if (verbose) {
            printf("%-8d %-6d %-8d %-6s %-8s %-6d %-6d\n",
                   vaddr, page_number, offset, tlb_status, pt_status,
                   frame, value);
        }
    }
    fclose(addr_fp);

    printf("\n=========================== RESULTS ========================\n");
    printf(" Total memory references : %ld\n", total_refs);
    printf(" TLB hits                : %lu\n", tlb.hits);
    printf(" TLB misses              : %lu\n", tlb.misses);
    if (total_refs > 0) {
        printf(" TLB hit rate            : %.2f%%\n",
               100.0 * tlb.hits / total_refs);
    }
    printf(" Page hits (in table)    : %lu\n", pt.page_hits);
    printf(" Page faults             : %lu\n", pt.page_faults);
    if (total_refs > 0) {
        printf(" Page fault rate         : %.2f%%\n",
               100.0 * pt.page_faults / total_refs);
    }
    printf(" Page evictions          : %lu\n", pt.evictions);
    printf("=============================================================\n");

    return EXIT_SUCCESS;
}
