#include "util/types.h"

// #define VGA_START 0xB8000
// #define VGA_EXTENT 80 * 25
//
// #define STYLE_WB 0x0F
//
// typedef struct __attribute__((packed)) {
//     char character;
//     char style;
// } vga_char;
//
// volatile vga_char *TEXT_AREA = (vga_char *)VGA_START;
//
// void clearwin() {
//     vga_char clear_char = {.character = ' ', .style = STYLE_WB};
//
//     for (unsigned int i = 0; i < VGA_EXTENT; i++) {
//         TEXT_AREA[i] = clear_char;
//     }
// }
//
// void putstr(const char *str) {
//     for (unsigned int i = 0; str[i] != '\0'; i++) {
//         if (i >= VGA_EXTENT)
//             break;
//
//         vga_char temp = {.character = str[i], .style = STYLE_WB};
//
//         TEXT_AREA[i] = temp;
//     }
// }
//
// __attribute__((noreturn)) void main(void) {
//     clearwin();
//
//     const char *welcome_msg =
//         "Welcome to the kernel! We can program in C and do debugging now";
//     putstr(welcome_msg);
//
//     while (1) {
//     }
// }
//

typedef struct {
    uint32_t columns;
    uint32_t rows;
    uint32_t scanline;
    uint64_t ptr;
    uint64_t size;
} FrameBuffer;

typedef struct {
    uint64_t ptr;
    uint64_t size;
} MemoryMap;

typedef struct __attribute__((packed)) {
    FrameBuffer fb;
    MemoryMap *memory;
} KernelParameters;

__attribute__((ms_abi)) int kernelmain(KernelParameters kernelParameters) {
    uint32_t *fb = (uint32_t *)kernelParameters.fb.ptr;
    uint32_t xres = kernelParameters.fb.scanline;
    uint32_t yres = kernelParameters.fb.columns;

    fb[0] = 0xFFDDDDDD;

    // Clear screen to solid color
    for (uint32_t y = 0; y < yres; y++)
        for (uint32_t x = 0; x < xres; x++)
            fb[y * xres + x] = 0xFFDDDDDD; // Light Gray AARRGGBB 8888

    // Draw square in top left
    for (uint32_t y = 0; y < yres / 5; y++)
        for (uint32_t x = 0; x < xres / 5; x++)
            fb[y * xres + x] = 0xFFCC2222; // AARRGGBB 8888

    while (1)
        ;
}
