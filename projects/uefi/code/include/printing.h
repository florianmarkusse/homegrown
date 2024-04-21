#ifndef PRINTING_H
#define PRINTING_H

#include "efi/c-efi-base.h"

void error(CEfiU16 *string);
void printNumber(CEfiUSize number, CEfiU8 base);
void printAsci(char *string);

#endif
