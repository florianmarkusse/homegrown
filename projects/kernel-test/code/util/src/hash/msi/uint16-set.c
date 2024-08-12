#include "util/hash/msi/uint16-set.h"
#include "util/hash/msi/common.h" // for indexLookup

bool msi_insertUint16(uint16_t value, size_t hash, msi_Uint16 *index) {
    for (int32_t i = (int32_t)hash;;) {
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

bool msi_containsUint16(uint16_t value, size_t hash, msi_Uint16 *index) {
    for (int32_t i = (int32_t)hash;;) {
        i = indexLookup(hash, index->exp, i);
        if (index->buf[i] == 0) {
            return false;
        } else if (index->buf[i] == value) {
            return true;
        }
    }
}
