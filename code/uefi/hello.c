#include <c-efi.h>

CEfiStatus efi_main(CEfiHandle h, CEfiSystemTable *st) {
    CEfiStatus r;

    r = st->con_out->output_string(st->con_out, L"Hello losers!\n");
    if (C_EFI_ERROR(r))
        return r;

    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS)
        ;

    return C_EFI_SUCCESS;
}
