#include "c-efi-base.h"                        // for CEfiStatus, C_EFI_SUC...
#include "c-efi-protocol-simple-text-input.h"  // for CEfiInputKey, CEfiSim...
#include "c-efi-protocol-simple-text-output.h" // for CEfiSimpleTextOutputP...
#include "c-efi-system.h"                      // for CEfiSystemTable

CEfiStatus efi_main([[__maybe_unused__]] CEfiHandle h, CEfiSystemTable *st) {
    CEfiStatus r;

    r = st->con_out->output_string(st->con_out, L"Hello losers!\n");
    if (C_EFI_ERROR(r))
        return r;

    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS)
        ;

    return C_EFI_SUCCESS;
}
