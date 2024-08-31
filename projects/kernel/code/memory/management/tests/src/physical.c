#include "test/physical.h"
#include "cpu/idt.h"
#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/memory/descriptor.h"
#include "log/log.h"
#include "memory/management/physical.h"
#include "test-framework/test.h"
#include "text/string.h"
#include "util/macros.h"
#include "util/maths.h"
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define TOTAL_BASE_PAGES (U64)(512 * 512 * 5)
#define MEMORY (PAGE_FRAME_SIZE * TOTAL_BASE_PAGES)

#define WITH_INIT_TEST(testString)                                             \
    resetTriggeredFaults();                                                    \
    TEST(testString)

#define EXPECT_NO_FAILURE                                                      \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[FAULT_NUMS];                                \
        TEST_FAILURE { appendInterrupts(expectedFaults); }                     \
        break;                                                                 \
    }

#define EXPECT_SINGLE_FAULT(expectedFault)                                     \
    if (__builtin_setjmp(jmp_buf)) {                                           \
        static bool expectedFaults[FAULT_NUMS];                                \
        expectedFaults[expectedFault] = true;                                  \
        if (compareInterrupts(expectedFaults)) {                               \
            testSuccess();                                                     \
            break;                                                             \
        } else {                                                               \
            TEST_FAILURE { appendInterrupts(expectedFaults); }                 \
            break;                                                             \
        }                                                                      \
    }

static PhysicalBasePage *memoryStart = NULL;
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
    initIDTTest(jmp_buf);

    PhysicalBasePage *pages = mmap(NULL, MEMORY, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pages == MAP_FAILED) {
        FLUSH_AFTER {
            LOG(STRING("Failed to allocate memory!\n"));
            LOG(STRING("Error code: "));
            LOG(errno, NEWLINE);
            LOG(STRING("Error message: "));
            LOG(STRING_LEN(strerror(errno), strlen(strerror(errno))), NEWLINE);
        }
        return;
    }

    // Setting the memoryStart to a page that is a power of a large page
    // Otherwise the math to do the tests get very wacky.
    memoryStart = (PhysicalBasePage *)(ALIGN_UP_EXP(
        (U64)pages, PAGE_FRAME_SHIFT + PAGE_TABLE_SHIFT * 2));

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

                FreeMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses,
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

                FreeMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses,
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
                                     PAGE_TABLE_ENTRIES - 1, &index),
                    createDescriptor(CONVENTIONAL_MEMORY, 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                FreeMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses,
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
                                     PAGE_TABLE_ENTRIES * 5 + 1, &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 4; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 510; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                FreeMemory memoryForAddresses[2];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses,
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
                        PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES - 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES,
                                     &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                for (U64 i = 0; i < 511; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 509; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                for (U64 i = 0; i < 100; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                for (U64 i = 0; i < 511; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        LARGE_PAGE);
                }

                for (U64 i = 0; i < 412; i++) {
                    FreeMemory memoryForAddresses[1];
                    allocPhysicalPages(
                        (FreeMemory_a){.buf = memoryForAddresses,
                                       .len = COUNTOF(memoryForAddresses)},
                        BASE_PAGE);
                }

                EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

                FreeMemory memoryForAddresses[2];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses,
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
                                     PAGE_TABLE_ENTRIES * 5 + 1, &index)};
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
                    (FreeMemory){.pageStart = 0, .numberOfPages = 1},
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
                        PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES - 3, &index),
                    createDescriptor(CONVENTIONAL_MEMORY,
                                     PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES,
                                     &index)};
                KernelMemory kernelMemory =
                    createKernelMemory(descriptors, COUNTOF(descriptors));

                EXPECT_NO_FAILURE;

                initPhysicalMemoryManager(kernelMemory);

                allocContiguousPhysicalPages(511, LARGE_PAGE);
                allocContiguousPhysicalPages(507, BASE_PAGE);
                allocContiguousPhysicalPages(2, BASE_PAGE);

                allocContiguousPhysicalPages(
                    PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES, BASE_PAGE);

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
                createDescriptor(CONVENTIONAL_MEMORY,
                                 PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES - 3,
                                 &index),
                createDescriptor(CONVENTIONAL_MEMORY,
                                 PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES,
                                 &index)};
            KernelMemory kernelMemory =
                createKernelMemory(descriptors, COUNTOF(descriptors));

            EXPECT_NO_FAILURE;

            initPhysicalMemoryManager(kernelMemory);

            allocContiguousPhysicalPages(511, LARGE_PAGE);

            for (U64 i = 0; i < 64; i++) {
                FreeMemory memoryForAddresses[8];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses,
                                   .len = COUNTOF(memoryForAddresses)},
                    LARGE_PAGE);
            }

            for (U64 i = 0; i < 509; i++) {
                FreeMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses, .len = 1},
                    BASE_PAGE);
            }

            freePhysicalPage((FreeMemory){.pageStart = 0, .numberOfPages = 100},
                             BASE_PAGE);

            for (U64 i = 0; i < 100; i++) {
                FreeMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses, .len = 1},
                    BASE_PAGE);
            }

            FreeMemory freePages[] = {
                (FreeMemory){.pageStart = 0, .numberOfPages = 1},
                (FreeMemory){.pageStart = 0, .numberOfPages = 2},
                (FreeMemory){.pageStart = 0, .numberOfPages = 3},
                (FreeMemory){.pageStart = 0, .numberOfPages = 4},
                (FreeMemory){.pageStart = 0, .numberOfPages = 5},
                (FreeMemory){.pageStart = 0, .numberOfPages = 6}};
            freePhysicalPages(
                (FreeMemory_a){.buf = freePages, .len = COUNTOF(freePages)},
                HUGE_PAGE);

            for (U64 i = 0; i < 21; i++) {
                FreeMemory memoryForAddresses[1];
                allocPhysicalPages(
                    (FreeMemory_a){.buf = memoryForAddresses, .len = 512},
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