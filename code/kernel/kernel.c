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
    vga_char *memory = (vga_char *)VGA_START;

    memory[0] = (vga_char){.character = 'j', .style = 0x0F};
    while (1) {
    }
}
