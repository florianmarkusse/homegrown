#include "gdt.h"

void setup_gdt() { load_gdt(sizeof(gdt_table) - 1, (CEfiU64)&gdt_table); }
