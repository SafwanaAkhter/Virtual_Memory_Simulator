/* ============================================================================
 * backing_store.h
 * ----------------------------------------------------------------------------
 * Models the disk / swap space that pages are loaded from on a page fault.
 * Implemented as a flat binary file of BACKING_STORE_SIZE bytes; page p's
 * content lives at byte offset (p * PAGE_SIZE) in the file.
 * ==========================================================================*/
#ifndef VMSIM_BACKING_STORE_H
#define VMSIM_BACKING_STORE_H

/* Ensures BACKING_STORE_FILE exists and is BACKING_STORE_SIZE bytes long,
 * generating deterministic pseudo-random content if it must be created. */
void backing_store_ensure_exists(void);

/* Reads exactly one page (PAGE_SIZE bytes) for page_number into dest. */
void backing_store_read_page(int page_number, unsigned char *dest);

#endif /* VMSIM_BACKING_STORE_H */
