#ifndef GLOBALS_H
#define GLOBALS_H

#include "efi/c-efi-base.h"

static CEfiHandle h;
static CEfiSystemTable *st;

static CEfiU64 *level4PageTable = C_EFI_NULL;

#endif
