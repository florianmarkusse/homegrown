#include "platform-abstraction/log.h"

U64 appendToBuffer(U8 *buffer, string data) {
    memcpy(buffer, data.buf, data.len);
    return data.len;
}
