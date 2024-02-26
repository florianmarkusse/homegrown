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

    int a[4646545646546565];

    FLO_SERIAL(FLO_STRING("Address of a is..."));
    FLO_SERIAL(a, FLO_NEWLINE);
    FLO_SERIAL(FLO_STRING("Attempting to print free memory...\n"));
    uint64_t totalSize = 0;
    MMapEnt *mmap_ent = &bootboot.mmap;
    while ((uint64_t)mmap_ent < (uint64_t)((char *)&bootboot + bootboot.size)) {
        FLO_SERIAL(FLO_STRING("Location: "));
        FLO_SERIAL(mmap_ent->ptr);
        FLO_SERIAL(FLO_STRING(" Location through interface : "));
        FLO_SERIAL(MMapEnt_Ptr(mmap_ent));

        FLO_SERIAL(FLO_STRING(" Size: "));
        FLO_SERIAL(mmap_ent->size);
        FLO_SERIAL(FLO_STRING(" Size through interface: "));
        FLO_SERIAL(MMapEnt_Size(mmap_ent));

        FLO_SERIAL(FLO_STRING(" type: "));
        FLO_SERIAL(MMapEnt_Type(mmap_ent));

        FLO_SERIAL(FLO_STRING(" isFre: "));
        FLO_SERIAL(MMapEnt_IsFree(mmap_ent), FLO_NEWLINE);

        totalSize += mmap_ent->size;
        mmap_ent++;
    }

    FLO_SERIAL(FLO_STRING("Total free bytes memory: "));
    FLO_SERIAL(totalSize, FLO_NEWLINE);

    FLO_LOG(FLO_STRING(
        "h\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh"
        "\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh\nh"));

    // hang for now
    while (1)
        ;
}
