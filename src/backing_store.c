/* ============================================================================
 * backing_store.c  -  see backing_store.h for module contract
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"
#include "../include/backing_store.h"

/* Creates a deterministic (seeded) pseudo-random backing store so that
 * repeated runs of the simulator are reproducible for grading/debugging. */
void backing_store_ensure_exists(void)
{
    FILE *fp = fopen(BACKING_STORE_FILE, "rb");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size == BACKING_STORE_SIZE) {
            return; /* already present and correctly sized */
        }
    }

    fp = fopen(BACKING_STORE_FILE, "wb");
    if (fp == NULL) {
        fprintf(stderr, "FATAL: cannot create backing store '%s'\n",
                BACKING_STORE_FILE);
        exit(EXIT_FAILURE);
    }

    srand(42); /* fixed seed -> reproducible content */
    unsigned char buffer[PAGE_SIZE];
    for (int page = 0; page < NUM_VIRTUAL_PAGES; page++) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            /* Fill each page with a recognisable pattern (page number in
             * high byte, pseudo-random low byte) so dumps are readable
             * during debugging. */
            buffer[i] = (unsigned char)((page * 7 + i + rand()) & 0xFF);
        }
        fwrite(buffer, 1, PAGE_SIZE, fp);
    }
    fclose(fp);
}

void backing_store_read_page(int page_number, unsigned char *dest)
{
    FILE *fp = fopen(BACKING_STORE_FILE, "rb");
    if (fp == NULL) {
        fprintf(stderr, "FATAL: cannot open backing store '%s'\n",
                BACKING_STORE_FILE);
        exit(EXIT_FAILURE);
    }
    long offset = (long)page_number * PAGE_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "FATAL: seek failed for page %d\n", page_number);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    size_t n = fread(dest, 1, PAGE_SIZE, fp);
    if (n != PAGE_SIZE) {
        fprintf(stderr, "FATAL: short read for page %d\n", page_number);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
}
