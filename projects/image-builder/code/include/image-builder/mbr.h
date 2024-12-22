#ifndef MBR_H
#define MBR_H

#include "posix/log.h"
#include "shared/types/types.h"
void writeMbr(WriteBuffer *file, U32 LBASize, U64 totalImageSize);
#endif
