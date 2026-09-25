/* ============================================================================
 * config.h
 * ----------------------------------------------------------------------------
 * Global, compile-time configuration for the Virtual Memory Simulator.
 *
 * Changing these constants lets the same code model different address-space
 * / physical-memory / TLB geometries without touching any other file, which
 * is the whole point of centralising them here instead of scattering magic
 * numbers through the modules.
 * ==========================================================================*/
#ifndef VMSIM_CONFIG_H
#define VMSIM_CONFIG_H

/* ---- Address space geometry ------------------------------------------- */
#define VIRTUAL_ADDR_BITS   16                 /* 16-bit virtual addresses   */
#define PAGE_OFFSET_BITS    8                   /* bits used for in-page off */
#define PAGE_SIZE           (1 << PAGE_OFFSET_BITS)      /* 256 bytes/page  */
#define NUM_VIRTUAL_PAGES   (1 << (VIRTUAL_ADDR_BITS - PAGE_OFFSET_BITS))
                                                 /* 65536/256 = 256 pages    */

/* ---- Physical memory geometry ------------------------------------------ */
#define NUM_PHYS_FRAMES     128                 /* deliberately < NUM_VIRTUAL_PAGES
                                                    so replacement is forced */
#define PHYS_MEM_SIZE       (NUM_PHYS_FRAMES * PAGE_SIZE)

/* ---- TLB geometry ------------------------------------------------------- */
#define TLB_SIZE            16                  /* fully-associative, small  */

/* ---- Backing store (simulated disk / swap space) ------------------------ */
#define BACKING_STORE_FILE  "data/BACKING_STORE.bin"
#define BACKING_STORE_SIZE  (NUM_VIRTUAL_PAGES * PAGE_SIZE)  /* 65536 bytes  */

/* ---- Replacement policy selectors --------------------------------------- */
typedef enum {
    POLICY_FIFO = 0,
    POLICY_LRU  = 1
} replace_policy_t;

#endif /* VMSIM_CONFIG_H */
