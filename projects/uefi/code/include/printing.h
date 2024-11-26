#ifndef PRINTING_H
#define PRINTING_H

#include "shared/types/types.h" // for U8, USize, U16

void error(U16 *string);
void printNumber(USize number, U8 base);
void printAsci(U8 *string);
void printAsciSize(U8 *string, USize size);

#endif
