#!/usr/bin/env python3
"""
generate_addresses.py
----------------------------------------------------------------------------
Produces a synthetic virtual-address reference trace that mimics real
program behaviour (spatial + temporal locality) instead of pure uniform
randomness, so that the simulator's TLB hit-rate and page-fault statistics
look like those of an actual process rather than a worst-case adversarial
trace.

Model: at each step, with probability P_LOCAL the next address stays within
a small "hot" window around the current address (models a tight loop /
sequential scan); otherwise it jumps to a new random hot window (models a
call into a different function / data structure).

Usage: python3 generate_addresses.py [count] > data/addresses.txt
"""
import random
import sys

VIRTUAL_ADDR_SPACE = 65536      # must match config.h: 2^16
WINDOW_SIZE         = 64        # size of a "hot" locality window
P_LOCAL             = 0.85      # probability of staying in the current window

def main():
    count = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
    random.seed(7)  # reproducible trace

    window_start = random.randint(0, VIRTUAL_ADDR_SPACE - WINDOW_SIZE)
    for _ in range(count):
        if random.random() > P_LOCAL:
            window_start = random.randint(0, VIRTUAL_ADDR_SPACE - WINDOW_SIZE)
        addr = window_start + random.randint(0, WINDOW_SIZE - 1)
        print(addr)

if __name__ == "__main__":
    main()
