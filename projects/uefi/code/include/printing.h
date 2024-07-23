#ifndef PRINTING_H
#define PRINTING_H

#include "efi/c-efi-base.h"

void error(U16 *string);
void printNumber(USize number, U8 base);
void printAsci(unsigned char *string);
void printAsciSize(unsigned char *string, USize size);

#endif
