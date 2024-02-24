#include "bootboot.h"
#include "types.h"
#include "util/assert.h"
#include "util/log.h"
#include "util/memory/memory.h"

/* imported virtual addresses, see linker script */
extern BOOTBOOT bootboot; // see bootboot.h
extern unsigned char
    environment[4096]; // configuration, UTF-8 text key=value pairs
extern uint8_t fb;     // linear framebuffer mapped

#define HAXOR_GREEN 0x0000FF00
#define BYTES_PER_PIXEL 4

/******************************************
 * Entry point, called by BOOTBOOT Loader *
 ******************************************/
void main() {
    /*** NOTE: this code runs on all cores in parallel ***/

    // TODO: When there is no screen attached this will be an issue.
    FLO_ASSERT(bootboot.fb_scanline);

    flo_setupScreen((flo_ScreenDimension){.buffer = &fb,
                                          .size = bootboot.fb_size,
                                          .width = bootboot.fb_width,
                                          .height = bootboot.fb_height,
                                          .scanline = bootboot.fb_scanline});

    int x, y, s = bootboot.fb_scanline, w = bootboot.fb_width,
              h = bootboot.fb_height;

    for (y = 0; y < w; y++) {
        *((uint32_t *)(&fb + y * BYTES_PER_PIXEL)) = HAXOR_GREEN;
    }
    for (y = 0; y < w; y++) {
        *((uint32_t *)(&fb + (h - 1) * s + y * BYTES_PER_PIXEL)) = HAXOR_GREEN;
    }
    for (x = 0; x < h; x++) {
        *((uint32_t *)(&fb + s * x)) = HAXOR_GREEN;
    }
    for (x = 0; x < h; x++) {
        *((uint32_t *)(&fb + s * x + (w - 1) * BYTES_PER_PIXEL)) = HAXOR_GREEN;
    }

    FLO_SERIAL(FLO_STRING("Attempting to print free memory...\n"));
    uint64_t totalSize = 0;
    MMapEnt *mmap_ent = &bootboot.mmap;
    while ((uint64_t)mmap_ent < (uint64_t)((char *)&bootboot + bootboot.size)) {
        FLO_SERIAL(FLO_STRING("Location: "));
        FLO_SERIAL(mmap_ent->ptr);
        FLO_SERIAL(FLO_STRING(" Size: "));
        FLO_SERIAL(mmap_ent->size, FLO_NEWLINE);

        totalSize += mmap_ent->size;
        mmap_ent++;
    }

    FLO_SERIAL(FLO_STRING("Total free bytes memory: "));
    FLO_SERIAL(totalSize, FLO_NEWLINE);

    // hang for now
    while (1)
        ;
}
