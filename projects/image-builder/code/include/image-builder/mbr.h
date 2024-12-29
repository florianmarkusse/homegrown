#ifndef MBR_H
#define MBR_H

#include "posix/log.h"
#include "shared/types/types.h"
void writeMBR(WriteBuffer *file, U32 LBASize, U64 totalImageSize);
#endif
