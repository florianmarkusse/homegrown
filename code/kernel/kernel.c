#define VGA_START 0xB8000
// #define VGA_EXTENT 80 * 25
//
// #define STYLE_WB 0x0F
//
typedef struct __attribute__((packed)) {
    char character;
    char style;
} vga_char;
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

__attribute__((noreturn)) void main(void) {
    *((int *)0xb8000) = 0x07690748;
    while (1) {
    }
}
