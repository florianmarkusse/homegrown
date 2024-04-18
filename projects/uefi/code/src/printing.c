#include "printing.h"
#include "efi/c-efi-protocol-simple-text-input.h"
#include "efi/c-efi-protocol-simple-text-output.h"
#include "efi/c-efi-system.h"
#include "globals.h"

void error(CEfiU16 *string) {
    st->con_out->output_string(st->con_out, string);
    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS) {
        ;
    }
    st->runtime_services->reset_system(C_EFI_RESET_SHUTDOWN, C_EFI_SUCCESS, 0,
                                       C_EFI_NULL);
}
