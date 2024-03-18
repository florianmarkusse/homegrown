#include <c-efi.h>

CEfiStatus efi_main(CEfiHandle h, CEfiSystemTable *st) {
    CEfiStatus r;
    CEfiUSize x;

    r = st->con_out->output_string(st->con_out, L"Hello World!\n");
    if (C_EFI_ERROR(r))
        return r;

    r = st->boot_services->wait_for_event(1, &st->con_in->wait_for_key, &x);
    if (C_EFI_ERROR(r))
        return r;

    return 0;
}
