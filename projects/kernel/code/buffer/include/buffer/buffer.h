#ifndef BUFFER_BUFFER_H
#define BUFFER_BUFFER_H

#include "interoperation/array-types.h"
#include "interoperation/types.h"
#include "memory/management/allocator/arena.h"
#include "text/string.h"

U64 appendToSimpleBuffer(string data, U8_d_a *array, Arena *perm);

#define APPEND_DATA(data, buffer, perm)                                        \
    appendToSimpleBuffer(CONVERT_TO_STRING(data), buffer, perm)

#endif
