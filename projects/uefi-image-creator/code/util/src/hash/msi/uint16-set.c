#include "util/hash/msi/uint16-set.h"
#include "interoperation/types.h"
#include "util/hash/msi/common.h" // for flo_indexLookup

bool flo_msi_insertUint16(U16 value, U64 hash, msi_U16 *index) {
    for (U32 i = (U32)hash;;) {
        i = flo_indexLookup(hash, index->exp, i);
        if (index->buf[i] == 0) {
            index->len++;
            index->buf[i] = value;
            return true;
        } else if (index->buf[i] == value) {
            return false;
        }
    }
}

bool flo_msi_containsUint16(U16 value, U64 hash, msi_U16 *index) {
    for (U32 i = (U32)hash;;) {
        i = flo_indexLookup(hash, index->exp, i);
        if (index->buf[i] == 0) {
            return false;
        } else if (index->buf[i] == value) {
            return true;
        }
    }
}
