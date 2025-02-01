#include "efi/error.h"
#include "efi/firmware/simple-text-input.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"

void waitKeyThenReset() {
    InputKey key;
    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
           SUCCESS) {
        ;
    }
    globals.st->runtime_services->reset_system(RESET_SHUTDOWN, SUCCESS, 0,
                                               nullptr);
}
