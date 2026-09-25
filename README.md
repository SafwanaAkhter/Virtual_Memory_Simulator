# Virtual Memory Simulator

A C-based simulation of how a virtual memory system works internally, including **page tables, Translation Lookaside Buffers (TLB), page faults, and page replacement**.

This project was developed as a systems programming / computer architecture project to make the concepts of virtual memory easier to understand by actually implementing and observing them in code.

Instead of only studying how the CPU and operating system handle memory, this simulator lets us see the complete address translation process and measure things such as TLB hit rate, page fault rate, and page evictions.

---

## 📌 Project Overview

Virtual memory allows a program to work with a large virtual address space even when the available physical memory is smaller.

This simulator models that process using:

* Page Table
* Translation Lookaside Buffer (TLB)
* Physical Memory
* Simulated Backing Store
* Page Fault Handling
* Page Replacement
* Virtual-to-Physical Address Translation

The simulator takes a stream of virtual memory addresses and processes them one by one.

For each address, it first checks the TLB. If the translation is not found there, it checks the page table. If the page is not currently in physical memory, a page fault occurs and the simulator loads the required page from the simulated backing store.

---

## 🎯 Main Objectives

The main goals of this project are to:

* Understand how virtual addresses are translated into physical addresses.
* Implement a single-level page table.
* Simulate a small, fully associative TLB.
* Handle page faults.
* Implement page replacement using **FIFO** and **LRU**.
* Keep the TLB and page table consistent after page eviction.
* Simulate a backing store for loading pages.
* Calculate useful performance statistics.
* Study how memory locality affects TLB hits and page faults.

---

## ⚙️ System Configuration

The simulator uses the following configuration:

| Parameter                 |        Value |
| ------------------------- | -----------: |
| Virtual address size      |      16 bits |
| Virtual address space     | 65,536 bytes |
| Page size                 |    256 bytes |
| Number of virtual pages   |          256 |
| Physical memory           | 32,768 bytes |
| Number of physical frames |          128 |
| TLB size                  |   16 entries |
| TLB replacement           |         FIFO |
| Page replacement          |   FIFO / LRU |
| Backing store             | 65,536 bytes |

Since the physical memory is smaller than the virtual address space, not all virtual pages can stay in memory at the same time. This allows the simulator to demonstrate page replacement and page faults.

---

## 🔄 How Address Translation Works

For every virtual address:

```text
Virtual Address
      │
      ▼
  Split into
 Page + Offset
      │
      ▼
   TLB Lookup
   /        \
 HIT        MISS
  │           │
  │           ▼
  │       Page Table
  │        Lookup
  │        /     \
  │      HIT     FAULT
  │       │        │
  │       │        ▼
  │       │   Load page from
  │       │   backing store
  │       │        │
  │       │        ▼
  │       │   Replace page
  │       │   if necessary
  │       │
  └───────┴────────┐
                   ▼
          Physical Address
```

The physical address is calculated using:

```text
Physical Address = Frame Number × Page Size + Offset
```

---

## 🧠 Page Replacement

When physical memory becomes full and a new page needs to be loaded, the simulator must decide which existing page should be removed.

Two replacement policies are implemented:

### FIFO — First-In, First-Out

The page that has been in memory for the longest time is selected for eviction.

### LRU — Least Recently Used

The page that has not been used for the longest time is selected for eviction.

The policy can be selected when running the simulator.

---

## 📂 Project Structure

```text
vmsim/
│
├── include/
│   ├── config.h
│   ├── tlb.h
│   ├── page_table.h
│   └── backing_store.h
│
├── src/
│   ├── main.c
│   ├── tlb.c
│   ├── page_table.c
│   └── backing_store.c
│
├── data/
│   └── addresses.txt
│
├── scripts/
│   └── generate_addresses.py
│
├── bin/
│
├── Makefile
└── README.md
```

### What each part does

**`config.h`**
Contains the main configuration values such as page size, number of physical frames, virtual address size, and TLB size.

**`tlb.c / tlb.h`**
Handles TLB lookup, insertion, FIFO replacement, and invalidation when a page is evicted.

**`page_table.c / page_table.h`**
Handles page-table operations, page faults, frame allocation, and FIFO/LRU page replacement.

**`backing_store.c / backing_store.h`**
Simulates secondary storage and provides page data when a page fault occurs.

**`main.c`**
Acts as the main driver. It reads virtual addresses, performs address translation, and displays the final statistics.

**`generate_addresses.py`**
Generates a synthetic address trace with locality, allowing the simulator to behave more like a real program's memory access pattern.

**`Makefile`**
Automates compilation and provides convenient commands for building and running the project.

---

## 🛠️ Requirements

To build and run the project, you need:

* GCC
* GNU Make
* C11-compatible compiler
* Python 3 *(only required for generating address traces)*

The project was developed and tested using GCC and GNU Make on a POSIX-like environment.

---

## 🚀 How to Build

Clone the repository:

```bash
git clone https://github.com/SafwanaAkhter/Virtual_Memory_Simulator.git
```

Move into the project directory:

```bash
cd Virtual_Memory_Simulator
```

Build the project:

```bash
make
```

This creates the executable:

```text
bin/vmsim
```

---

## ▶️ Running the Simulator

### Run using LRU

```bash
./bin/vmsim data/addresses.txt lru
```

### Run using FIFO

```bash
./bin/vmsim data/addresses.txt fifo
```

### Run with detailed output

```bash
./bin/vmsim data/addresses.txt lru -v
```

The verbose mode displays information for each memory reference, including:

* Virtual address
* Page number
* Offset
* TLB status
* Page-table status
* Physical frame
* Retrieved value

---

## 📊 Example Results

Using the provided 2,000-address locality-aware trace with **LRU**, the simulator produced:

```text
Total memory references : 2000
TLB hits                : 1667
TLB misses              : 333
TLB hit rate            : 83.35%

Page hits               : 107
Page faults             : 226
Page fault rate         : 11.30%
Page evictions          : 98
```

These results show that most memory references were served through the TLB, while the remaining references required a page-table lookup.

For comparison, running the same trace with FIFO produced:

```text
TLB hit rate   : 83.40%
Page fault rate: 11.10%
Page evictions : 94
```

The project uses the same input trace for both policies so their behavior can be compared under the same workload.

---

## 🧪 Testing

The simulator was tested using several different scenarios:

### 1. Normal memory-access trace

A 2,000-reference trace was used to test normal address translation and replacement behavior.

### 2. Low-locality stress test

A 5,000-reference random address trace was used to put more pressure on the TLB and physical memory.

The stress test produced a much lower TLB hit rate and a higher page-fault rate, demonstrating how strongly memory locality affects virtual-memory performance.

### 3. Invalid addresses

The simulator checks addresses before using them. Invalid addresses such as values outside the 16-bit address range are skipped with a warning instead of causing invalid memory access.

### 4. Memory-safety testing

The project was also tested using GCC AddressSanitizer and UndefinedBehaviorSanitizer. The reported test runs completed without sanitizer errors.

---

## 📈 What This Project Demonstrates

This project helped demonstrate several important concepts in memory management:

* How virtual addresses are divided into page numbers and offsets.
* How a TLB speeds up address translation.
* What happens during a TLB hit and TLB miss.
* How page tables map virtual pages to physical frames.
* How page faults are handled.
* How pages are loaded from secondary storage.
* How FIFO and LRU replacement policies work.
* Why TLB/page-table consistency is important after page eviction.
* How locality of reference affects memory-system performance.

---

## ⚠️ Current Limitations

This simulator intentionally keeps the design relatively simple so that the main virtual-memory concepts remain easy to understand.

Current limitations include:

* Only a single-level page table is implemented.
* Only one simulated address space/process is supported.
* Victim selection uses a linear scan.
* Dirty pages are tracked but are not written back to the backing store during eviction.
* The TLB uses FIFO replacement only.
* It does not model multiple processes or context switching.

These limitations are part of the current project scope rather than bugs in the implementation.

---

## 🔮 Possible Future Improvements

Some possible extensions include:

* Multi-level or inverted page tables.
* Clock / Second-Chance page replacement.
* More efficient LRU implementation.
* Write-back support for dirty pages.
* Multiple simulated processes.
* Per-process or tagged TLB entries.
* Additional replacement algorithms such as Optimal and LFU.
* A graphical interface for visualizing page-table and TLB activity.
* Testing with real memory-reference traces instead of only synthetic traces.

These improvements would make the simulator closer to the behavior of a real operating-system memory manager.

---

## 📚 References

The theoretical concepts used in this project were informed by standard operating-systems and systems-programming references, including:

* Silberschatz, Galvin & Gagne — *Operating System Concepts*
* Tanenbaum & Bos — *Modern Operating Systems*
* Robert Love — *Linux Kernel Development*
* Peter Denning — *Virtual Memory*
* Belady — *A Study of Replacement Algorithms for a Virtual-Storage Computer*
* Intel 64 and IA-32 Architectures Software Developer's Manual
* GCC Documentation
* Kernighan & Ritchie — *The C Programming Language*

---

## 👩‍💻 Project

**Virtual Memory Management Simulator**

Built with:

```text
C / C11
GCC
GNU Make
Python 3
```

The project was developed as a practical implementation of virtual memory concepts, with the goal of connecting operating-system theory with actual working code.
