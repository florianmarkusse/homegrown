#include "cpu/idt.h"
#include "interoperation/array.h"
#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/memory/descriptor.h"
#include "log/log.h"
#include "memory/management/physical.h"
#include "test-framework/test.h"
#include "text/string.h"
#include "util/macros.h"
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define TOTAL_BASE_PAGES 512 * 512
#define MEMORY PAGE_FRAME_SIZE *TOTAL_BASE_PAGES

bool compareInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();

    for (U64 i = 0; i < FAULT_NUMS; i++) {
        if (expectedFaults[i] != actualFaults[i]) {
            return false;
        }
    }

    return true;
}

void appendInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();
    LOG(STRING("Interrupts Table\n"));
    for (U64 i = 0; i < FAULT_NUMS; i++) {
        LOG(STRING("Fault #: "));
        LOG(i);
        LOG(STRING("\tMsg: "));
        LOG(stringWithMinSizeDefault(faultToString(i), 30));
        LOG(STRING("\tExpected: "));
        LOG(stringWithMinSizeDefault(
            expectedFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        LOG(STRING("\tActual: "));
        LOG(stringWithMinSizeDefault(
            actualFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        if (expectedFaults[i] != actualFaults[i]) {
            LOG(STRING("\t!!!"));
        }
        LOG(STRING("\n"));
    }
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

MemoryDescriptor createDescriptor(MemoryType type, PhysicalBasePage *address,
                                  U64 numberOfPages) {
    return (MemoryDescriptor){.type = type,
                              .attribute = 0,
                              .virtualStart = (U64)address,
                              .physicalStart = (U64)address,
                              .numberOfPages = numberOfPages};
}

int main() {
    testSuiteStart(STRING("Memory Management"));

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
        return -1;
    }

    TEST_TOPIC(STRING("Physical Memory Management")) {
        TEST(STRING("No available physical memory")) {
            MemoryDescriptor descriptors[] = {
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 100),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1)};
            KernelMemory kernelMemory = {
                .totalDescriptorSize =
                    sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                .descriptorSize = sizeof(MemoryDescriptor),
                .descriptors = descriptors};

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            initPhysicalMemoryManager(kernelMemory);

            TEST_FAILURE {
                static bool expectedFaults[FAULT_NUMS];
                FLUSH_AFTER { appendInterrupts(expectedFaults); }
            }
        }

        TEST(STRING("Too little available physical memory")) {
            MemoryDescriptor descriptors[] = {
                createDescriptor(CONVENTIONAL_MEMORY, pages, 2),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1)};
            KernelMemory kernelMemory = {
                .totalDescriptorSize =
                    sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                .descriptorSize = sizeof(MemoryDescriptor),
                .descriptors = descriptors};

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            initPhysicalMemoryManager(kernelMemory);

            TEST_FAILURE {
                static bool expectedFaults[FAULT_NUMS];
                FLUSH_AFTER { appendInterrupts(expectedFaults); }
            }
        }

        TEST(STRING("Single region of 3 memory pages")) {
            MemoryDescriptor descriptors[] = {
                createDescriptor(CONVENTIONAL_MEMORY, pages, 3),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1)};
            KernelMemory kernelMemory = {
                .totalDescriptorSize =
                    sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                .descriptorSize = sizeof(MemoryDescriptor),
                .descriptors = descriptors};

            if (__builtin_setjmp(jmp_buf)) {
                TEST_FAILURE {
                    static bool expectedFaults[FAULT_NUMS];
                    FLUSH_AFTER { appendInterrupts(expectedFaults); }
                }
                break;
            }

            initPhysicalMemoryManager(kernelMemory);

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            allocPhysicalPages(
                (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}}, .len = 1},
                BASE_PAGE);

            testSuccess();
        }

        TEST(STRING("3 regions of single page memory")) {
            MemoryDescriptor descriptors[] = {
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 0, 1),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 1, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 2, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1),
                createDescriptor(RESERVED_MEMORY_TYPE, NULL, 1)};
            KernelMemory kernelMemory = {
                .totalDescriptorSize =
                    sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                .descriptorSize = sizeof(MemoryDescriptor),
                .descriptors = descriptors};

            if (__builtin_setjmp(jmp_buf)) {
                TEST_FAILURE {
                    static bool expectedFaults[FAULT_NUMS];
                    FLUSH_AFTER { appendInterrupts(expectedFaults); }
                }
                break;
            }

            initPhysicalMemoryManager(kernelMemory);

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            allocPhysicalPages(
                (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}}, .len = 1},
                BASE_PAGE);

            testSuccess();
        }

        TEST(STRING("Multiple regions of memory")) {
            MemoryDescriptor descriptors[] = {
                createDescriptor(CONVENTIONAL_MEMORY, pages + 0, 3),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 3, 500),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 503, 520),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 1024,
                                 PAGE_TABLE_ENTRIES - 1),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 5, 1)};
            KernelMemory kernelMemory = {
                .totalDescriptorSize =
                    sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                .descriptorSize = sizeof(MemoryDescriptor),
                .descriptors = descriptors};

            if (__builtin_setjmp(jmp_buf)) {
                TEST_FAILURE {
                    static bool expectedFaults[FAULT_NUMS];
                    FLUSH_AFTER { appendInterrupts(expectedFaults); }
                }
                break;
            }

            initPhysicalMemoryManager(kernelMemory);

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            allocPhysicalPages(
                (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}}, .len = 1},
                LARGE_PAGE);

            testSuccess();
        }

        TEST(STRING("Multiple PMMs")) {
            MemoryDescriptor descriptors[] = {
                createDescriptor(CONVENTIONAL_MEMORY, pages + 0, 3),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 3, 500),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 503, 521),
                createDescriptor(CONVENTIONAL_MEMORY, pages + 1024,
                                 PAGE_TABLE_ENTRIES * 5 + 1)};
            KernelMemory kernelMemory = {
                .totalDescriptorSize =
                    sizeof(MemoryDescriptor) * COUNTOF(descriptors),
                .descriptorSize = sizeof(MemoryDescriptor),
                .descriptors = descriptors};

            if (__builtin_setjmp(jmp_buf)) {
                TEST_FAILURE {
                    static bool expectedFaults[FAULT_NUMS];
                    FLUSH_AFTER { appendInterrupts(expectedFaults); }
                }
                break;
            }

            initPhysicalMemoryManager(kernelMemory);

            printPhysicalMemoryManagerStatus();

            for (U64 i = 0; i < 4; i++) {
                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    LARGE_PAGE);
            }

            for (U64 i = 0; i < 510; i++) {
                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    BASE_PAGE);
            }

            allocPhysicalPages(
                (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}}, .len = 1},
                LARGE_PAGE);

            printPhysicalMemoryManagerStatus();

            for (U64 i = 0; i < 512; i++) {
                allocPhysicalPages(
                    (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}},
                                   .len = 1},
                    BASE_PAGE);
            }

            EXPECT_SINGLE_FAULT(FAULT_NO_MORE_PHYSICAL_MEMORY);

            allocPhysicalPages(
                (FreeMemory_a){.buf = (FreeMemory[]){(FreeMemory){}}, .len = 1},
                BASE_PAGE);

            testSuccess();

            printPhysicalMemoryManagerStatus();
        }
    }

    return testSuiteFinish();
}
