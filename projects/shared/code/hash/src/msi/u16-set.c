#include "shared/hash/msi/u16-set.h"
#include "shared/hash/msi/common.h" // for indexLookup

bool msi_insertU16(U16 value, U64 hash, msi_U16 *index) {
    for (U32 i = (U32)hash;;) {
        i = indexLookup(hash, index->exp, i);
        if (index->buf[i] == 0) {
            index->len++;
            index->buf[i] = value;
            return true;
        } else if (index->buf[i] == value) {
            return false;
        }
    }
}

bool msi_containsU16(U16 value, U64 hash, msi_U16 *index) {
    for (U32 i = (U32)hash;;) {
        i = indexLookup(hash, index->exp, i);
        if (index->buf[i] == 0) {
            return false;
        } else if (index->buf[i] == value) {
            return true;
        }
    }
}
