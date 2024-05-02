#ifndef MEMORY_DEFINITIONS_H
#define MEMORY_DEFINITIONS_H

#define STACK_SIZE (1 << 14)
#define RED_ZONE_SIZE (1 << 7)

#define KERNEL_SPACE_START 0xffff800000000000
#define KERNEL_SPACE_END 0xffffffffffffffff

#define KERNEL_START 0xfffffffff8000000
#define BOTTOM_STACK KERNEL_START - STACK_SIZE
#define KERNEL_PARAMS_START 0xfffffffff7000000

#define PAGE_ENTRY_SHIFT 9
#define PAGE_ENTRY_SIZE (1 << PAGE_ENTRY_SHIFT)
#define PAGE_ENTRY_MASK (PAGE_ENTRY_SIZE - 1)

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK (PAGE_SIZE - 1)

#define PAGE_ENTRIES_SIZE_BYTES (8)
#define PAGE_ENTRIES_NUM (PAGE_SIZE / PAGE_ENTRIES_SIZE_BYTES)

#define PAGE_PRESENT (1ULL << 0)  // The page is currently in memory
#define PAGE_WRITABLE (1ULL << 1) // It’s allowed to write to this page
#define PAGE_USER_ACCESSIBLE                                                   \
    (1ULL << 2) // If not set, only kernel mode code can access this page
#define PAGE_WRITE_THROUGH (1ULL << 3) // Writes go directly to memory
#define PAGE_DISABLE_CACHE (1ULL << 4) // No cache is used for this page
#define PAGE_ACCESSED                                                          \
    (1ULL << 5) // The CPU sets this bit when this page is used
#define PAGE_DIRTY                                                             \
    (1ULL << 6) // The CPU sets this bit when a write to this page occurs
#define PAGE_HUGE_PAGE                                                         \
    (1ULL << 7) // Must be 0 in P1 and P4, creates a 1 GiB page in P3, creates a
                // 2 MiB page in P2
#define PAGE_GLOBAL                                                            \
    (1ULL << 8) // Page isn’t flushed from caches on address space switch (PGE
                // bit of CR4 register must be set)
#define PAGE_AVAILABLE_9 (1ULL << 9)   // Can be used freely by the OS
#define PAGE_AVAILABLE_10 (1ULL << 10) // Can be used freely by the OS
#define PAGE_AVAILABLE_11 (1ULL << 11) // Can be used freely by the OS
#define PAGE_PHYSICAL_ADDR_MASK                                                \
    0x7FFFFFFFFFFFFULL                 // Mask for the 52-bit physical address
#define PAGE_AVAILABLE_52 (1ULL << 52) // Can be used freely by the OS
#define PAGE_AVAILABLE_53 (1ULL << 53) // Can be used freely by the OS
#define PAGE_AVAILABLE_54 (1ULL << 54) // Can be used freely by the OS
#define PAGE_AVAILABLE_55 (1ULL << 55) // Can be used freely by the OS
#define PAGE_AVAILABLE_56 (1ULL << 56) // Can be used freely by the OS
#define PAGE_AVAILABLE_57 (1ULL << 57) // Can be used freely by the OS
#define PAGE_AVAILABLE_58 (1ULL << 58) // Can be used freely by the OS
#define PAGE_AVAILABLE_59 (1ULL << 59) // Can be used freely by the OS
#define PAGE_AVAILABLE_60 (1ULL << 60) // Can be used freely by the OS
#define PAGE_AVAILABLE_61 (1ULL << 61) // Can be used freely by the OS
#define PAGE_AVAILABLE_62 (1ULL << 62) // Can be used freely by the OS
#define PAGE_NO_EXECUTE                                                        \
    (1ULL << 63) // Forbid executing code on this page (the NXE bit in the EFER
                 // register must be set)

#define EFI_SIZE_TO_PAGES(a) (((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

#endif
