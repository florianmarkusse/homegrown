#include "test/physical.h"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#include "efi-to-kernel/kernel-parameters.h"
#include "efi-to-kernel/memory/descriptor.h"
#include "posix/log.h"
#include "posix/test-framework/test.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/maths/maths.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86/cpu/mock/idt.h"
#include "x86/cpu/status/test.h"
#include "x86/memory/physical.h"
#include "shared/memory/management/definitions.h"
#include "x86/cpu/fault.h"
#include "x86/memory/definitions/virtual.h"

static constexpr auto TOTAL_BASE_PAGES = (U64)(512 * 512 * 5);
static constexpr auto MEMORY = (PAGE_FRAME_SIZE * TOTAL_BASE_PAGES);

#define WITH_INIT_TEST(testString)                                             \
    resetTriggeredFaults();                                                    \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        TEST_FAILURE {                                                         \
            PLOG(                                                              \
                STRING(                                                        \
                    "Interrupt Jumper was not set up yet! Killing test loop"), \
                NEWLINE);                                                      \
        }                                                                      \
        break;                                                                 \
    }                                                                          \
    TEST(testString)

#define EXPECT_NO_FAILURE                                                      \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[CPU_FAULT_COUNT];                           \
        TEST_FAILURE {                                                         \
            appendInterrupts(expectedFaults, getTriggeredFaults());            \
        }                                                                      \
        break;                                                                 \
    }

#define EXPECT_SINGLE_FAULT(expectedFault)                                     \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[CPU_FAULT_COUNT];                           \
        expectedFaults[expectedFault] = true;                                  \
        if (compareInterrupts(expectedFaults)) {                               \
            testSuccess();                                                     \
            break;                                                             \
        } else {                                                               \
            TEST_FAILURE {                                                     \
                appendInterrupts(expectedFaults, getTriggeredFaults());        \
            }                                                                  \
            break;                                                             \
        }                                                                      \
    }

static PhysicalBasePage *memoryStart = nullptr;
MemoryDescriptor createDescriptor(MemoryType type, U64 numberOfPages,
                                  U64 *index) {
    U64 indexToUse = *index;

    *index += numberOfPages;
    return (MemoryDescriptor){.type = type,
                              .attribute = 0,
                              .virtualStart = (U64)(memoryStart + indexToUse),
                              .physicalStart = (U64)(memoryStart + indexToUse),
                              .numberOfPages = numberOfPages};
}

KernelMemory createKernelMemory(MemoryDescriptor *descriptors, U64 elements) {
    KernelMemory kernelMemory = {.totalDescriptorSize =
                                     sizeof(MemoryDescriptor) * elements,
                                 .descriptorSize = sizeof(MemoryDescriptor),
                                 .descriptors = descriptors};
    return kernelMemory;
}

void testPhysicalMemoryManagement() {
    void *jmp_buf[5];
    if (__builtin_setjmp(jmp_buf)) {
        TEST_FAILURE {
            PLOG(STRING("Interrupt Jumper was not set up yet!"), NEWLINE);
        }
        return;
    }

    initIDTTest(jmp_buf);

    PhysicalBasePage *pages = mmap(nullptr, MEMORY, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pages == MAP_FAILED) {
        PFLUSH_AFTER(STDERR) {
            PLOG(STRING("Failed to allocate memory!\n"));
            PLOG(STRING("Error code: "));
            PLOG(errno, NEWLINE);
            PLOG(STRING("Error message: "));
            PLOG(STRING_LEN(strerror(errno), strlen(strerror(errno))), NEWLINE);
        }
        return;
    }

    memoryStart = (PhysicalBasePage *)ALIGN_UP_VALUE(
        (U64)pages, PAGE_FRAME_SIZE * 2 * PageTableFormat.ENTRIES);

    TEST_TOPIC(STRING("Physical Memory Management")) {
        TEST_TOPIC(STRING("Initing")) {
            WITH_INIT_TEST(STRING("No available physical memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(RESERVED_MEMORY_TYPE, 100, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                initPhysicalMemoryManager(kernelMemory);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
            WITH_INIT_TEST(STRING("Too little available physical memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 2, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                initPhysicalMemoryManager(kernelMemory);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
            WITH_INIT_TEST(STRING("Single region of 3 memory pages")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("3 regions of single page memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index),
                    createDescriptor(RESERVED_MEMORY_TYPE, 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("Multiple regions of memory")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 500, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 520, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PageTableFormat.ENTRIES - 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    LARGE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Incontiguous allocation")) {
            WITH_INIT_TEST(STRING("Single-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 500, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 521, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PageTableFormat.ENTRIES * 5 + 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 4; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 510; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[2];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("Multi-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(
                        CONVENTIONAL_MEMORY,
                        PageTableFormat.ENTRIES * PageTableFormat.ENTRIES - 3,
                        &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PageTableFormat.ENTRIES *
                                         PageTableFormat.ENTRIES,
                                     &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 511; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 509; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                for (U64 i = 0; i < 100; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 412; i++) {
                    PagedMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (PagedMemory_a){.buf = memoryForAddresses,
                                        .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                PagedMemory memoryForAddresses[2];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    HUGE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        TEST_TOPIC(STRING("Contiguous allocation")) {
            WITH_INIT_TEST(STRING("Single-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 500, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 521, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PageTableFormat.ENTRIES * 5 + 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                allocContiguousPhysicalPages(498, BASE_PAGE);
                allocContiguousPhysicalPages(9, BASE_PAGE);
                allocContiguousPhysicalPages(2, BASE_PAGE);
                allocContiguousPhysicalPages(1, BASE_PAGE);

                allocContiguousPhysicalPages(500, BASE_PAGE);
                allocContiguousPhysicalPages(512 * 5, BASE_PAGE);
                allocContiguousPhysicalPages(12, BASE_PAGE);

                freePhysicalPage(
                    (PagedMemory){.pageStart = 0, .numberOfPages = 1},
                    BASE_PAGE);

                allocContiguousPhysicalPages(1, BASE_PAGE);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocContiguousPhysicalPages(1, BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }

            WITH_INIT_TEST(STRING("Multi-level stealing")) {
                U64 index = 0;
                MemoryDescriptor descriptors[] = {
                    createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                    createDescriptor(
                        CONVENTIONAL_MEMORY,
                        PageTableFormat.ENTRIES * PageTableFormat.ENTRIES - 3,
                        &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PageTableFormat.ENTRIES *
                                         PageTableFormat.ENTRIES,
                                     &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                allocContiguousPhysicalPages(511, LARGE_PAGE);
                allocContiguousPhysicalPages(507, BASE_PAGE);
                allocContiguousPhysicalPages(2, BASE_PAGE);

                allocContiguousPhysicalPages(PageTableFormat.ENTRIES *
                                                 PageTableFormat.ENTRIES,
                                             BASE_PAGE);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                allocContiguousPhysicalPages(1, BASE_PAGE);

                TEST_FAILURE {
                    appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
                }
            }
        }

        WITH_INIT_TEST(
            STRING("Combining contiguous and incontiguous allocation")) {
            U64 index = 0;
            MemoryDescriptor descriptors[] = {
                createDescriptor(CONVENTIONAL_MEMORY, 3, &index),
                createDescriptor(
                    CONVENTIONAL_MEMORY,
                    PageTableFormat.ENTRIES * PageTableFormat.ENTRIES - 3,
                    &index),
                createDescriptor(
                    CONVENTIONAL_MEMORY,
                    PageTableFormat.ENTRIES * PageTableFormat.ENTRIES, &index)};
            KernelMemory kernelMemory =
                createKernelMemory(descriptors, COUNTOF(descriptors));

            EXPECT_NO_FAILURE;

            initPhysicalMemoryManager(kernelMemory);

            allocContiguousPhysicalPages(511, LARGE_PAGE);

            for (U64 i = 0; i < 64; i++) {
                PagedMemory memoryForAddresses[8];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses,
                                    .len = COUNTOF(memoryForAddresses)},
                    LARGE_PAGE);
            }

            for (U64 i = 0; i < 509; i++) {
                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses, .len = 1},
                    BASE_PAGE);
            }

            freePhysicalPage(
                (PagedMemory){.pageStart = 0, .numberOfPages = 100}, BASE_PAGE);

            for (U64 i = 0; i < 100; i++) {
                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses, .len = 1},
                    BASE_PAGE);
            }

            PagedMemory freePages[] = {
                (PagedMemory){.pageStart = 0, .numberOfPages = 1},
                (PagedMemory){.pageStart = 0, .numberOfPages = 2},
                (PagedMemory){.pageStart = 0, .numberOfPages = 3},
                (PagedMemory){.pageStart = 0, .numberOfPages = 4},
                (PagedMemory){.pageStart = 0, .numberOfPages = 5},
                (PagedMemory){.pageStart = 0, .numberOfPages = 6}};
            freePhysicalPages(
                (PagedMemory_a){.buf = freePages, .len = COUNTOF(freePages)},
                HUGE_PAGE);

            for (U64 i = 0; i < 21; i++) {
                PagedMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (PagedMemory_a){.buf = memoryForAddresses, .len = 512},
                    LARGE_PAGE);
            }

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            allocContiguousPhysicalPages(1, BASE_PAGE);

            TEST_FAILURE {
                appendExpectedInterrupt(FAULT_NO_MORE_PHYSICAL_MEMORY);
            }
        }
    }
}
