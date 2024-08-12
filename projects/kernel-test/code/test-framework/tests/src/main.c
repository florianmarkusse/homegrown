#include "test-framework/test.h" // for testSuccess, TEST, TEST...
#include "util/log.h"            // for ERROR, LOG_CHOOSER_IMPL_2
#include "util/memory/arena.h"   // for arena
#include "util/memory/macros.h"  // for COUNTOF
#include "util/text/string.h"    // for STRING, string
#include <errno.h>               // for errno
#include <stddef.h>              // for NULL, ptrdiff_t
#include <string.h>              // for strerror
#include <sys/mman.h>            // for mmap, munmap, MAP_ANONYMOUS, MAP_FA...

#define CAP 1 << 21

static string testNames[] = {
    STRING("aaaaaaaaa"),    STRING("bbbbbbbbb"),  STRING("cccccccccc"),
    STRING("dddddddddd"),   STRING("eeeeeeeeee"), STRING("fffffffff"),
    STRING("ggggggggggg"),  STRING("hhhhhhhhh"),  STRING("iiiiiiiiii"),
    STRING("ffffffffffff"),
};

static ptrdiff_t numTestNames = COUNTOF(testNames);

void test1() {
    TEST(STRING("Test 1")) {
        //
        testSuccess();
    }
}

void test2() {
    TEST(STRING("Test 2")) {
        //
        testSuccess();
    }
}

void multipleTests() {
    for (ptrdiff_t i = 0; i < numTestNames; i++) {
        TEST(testNames[i]) {
            if (i % 2 == 0) {
                testSuccess();
            } else {
                TEST_FAILURE {
                    // Inside this scoped block you can do your
                    // additional logging.
                    ERROR("big chaos\n");
                }
            }
        }
    }
}

void test3() {
    TEST(STRING("Test 3")) {
        //
        TEST_FAILURE {
            // Inside this scoped block you can do your
            // additional logging.
            ERROR("biger ddfd chaos\n");
        }
    }
}

int main() {
    char *begin = mmap(NULL, CAP, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (begin == MAP_FAILED) {
        FLUSH_AFTER(STDERR) {
            ERROR("Failed to allocate memory!\n");
            ERROR("Error code: ");
            ERROR(errno, NEWLINE);
            ERROR("Error message: ");
            ERROR(strerror(errno), NEWLINE);
        }
        return -1;
    }

    Arena arena =
        (Arena){.beg = begin, .cap = CAP, .end = begin + (ptrdiff_t)(CAP)};

    void *jmp_buf[5];
    if (__builtin_setjmp(jmp_buf)) {
        if (munmap(arena.beg, arena.cap) == -1) {
            FLUSH_AFTER(STDERR) {
                ERROR((STRING("Failed to unmap memory from arena!\n"
                              "Arena Details:\n"
                              "  beg: ")));
                ERROR(arena.beg);
                ERROR((STRING("\n end: ")));
                ERROR(arena.end);
                ERROR((STRING("\n cap: ")));
                ERROR(arena.cap);
                ERROR((STRING("\nZeroing Arena regardless.\n")));
            }
        }
        arena.beg = NULL;
        arena.end = NULL;
        arena.cap = 0;
        arena.jmp_buf = NULL;
        ERROR((STRING("OOM/overflow in arena!\n")), FLUSH);
        return -1;
    }
    arena.jmp_buf = jmp_buf;

    testSuiteStart();

    TEST_TOPIC(STRING("My first topic")) { test1(); }

    TEST_TOPIC(STRING("My second topic")) {
        test2();
        TEST_TOPIC(STRING("inside topic")) { multipleTests(); }
        test3();
    }

    return testSuiteFinish();
}
