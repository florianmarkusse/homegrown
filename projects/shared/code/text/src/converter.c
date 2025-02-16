#include "shared/text/converter.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/assert.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h"

static constexpr auto STRING_CONVERTER_BUF_LEN = 1 * KiB;
static U8_a stringConverterBuffer = (U8_a){
    .buf = (U8[STRING_CONVERTER_BUF_LEN]){0}, .len = STRING_CONVERTER_BUF_LEN};

string stringToString(string data) { return data; }

string charToString(char data, U8_a tmp) {
    tmp.buf[0] = data;
    return (string){.len = 1, .buf = tmp.buf};
}

string charToStringDefault(char data) {
    return charToString(data, stringConverterBuffer);
}

string boolToString(bool data) {
    return (data ? STRING("true") : STRING("false"));
}

static U8 hexString[] = "0123456789ABCDEF";
string ptrToString(void *data, U8_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    U64 counter = 2;
    U64 u = (U64)data;
    for (int i = 2 * sizeof(u) - 1; i >= 0; i--) {
        tmp.buf[counter++] = hexString[(u >> (4 * i)) & 15];
    }

    return (string){.len = counter - 1, .buf = tmp.buf};
}

string ptrToStringDefault(void *data) {
    return ptrToString(data, stringConverterBuffer);
}

string U64ToString(U64 data, U8_a tmp) {
    U8 *end = tmp.buf + tmp.len;
    U8 *beg = end;
    do {
        *--beg = '0' + (U8)(data % 10);
    } while (data /= 10);
    return (STRING_PTRS(beg, end));
}

string U64ToStringDefault(U64 data) {
    return U64ToString(data, stringConverterBuffer);
}

string I64ToString(I64 data, U8_a tmp) {
    U8 *end = tmp.buf + tmp.len;
    U8 *beg = end;
    I64 t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (U8)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return STRING_PTRS(beg, end);
}

string I64ToStringDefault(I64 data) {
    return I64ToString(data, stringConverterBuffer);
}

#ifndef NO_FLOAT
string F64ToString(F64 data, U8_a tmp) {
    U64 tmpLen = 0;
    U32 prec = 1000000; // i.e. 6 decimals

    if (data < 0) {
        tmp.buf[tmpLen++] = '-';
        data = -data;
    }

    data += 0.5 / ((F64)prec);      // round last decimal
    if (data >= (F64)(-1UL >> 1)) { // out of long range?
        tmp.buf[tmpLen++] = 'i';
        tmp.buf[tmpLen++] = 'n';
        tmp.buf[tmpLen++] = 'f';
        return STRING_LEN(tmp.buf, tmpLen);
    }

    U64 integral = (U64)data;
    U64 fractional = (U64)((data - (F64)integral) * (F64)prec);

    U8 buf2[64];
    U8_a tmp2 = (U8_a){.buf = buf2, .len = 64};

    string part = U64ToString(integral, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    tmp.buf[tmpLen++] = '.';

    U8 counter = 0;
    for (U32 i = prec / 10; i > 1; i /= 10) {
        if (i > fractional) {
            counter++;
        }
    }
    memset(tmp.buf + tmpLen, '0', counter);

    part = U64ToString(fractional, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    return STRING_LEN(tmp.buf, tmpLen);
}

string F64ToStringDefault(F64 data) {
    return F64ToString(data, stringConverterBuffer);
}
#endif

string stringWithMinSize(string data, U8 minSize, U8_a tmp) {
    if (data.len >= minSize) {
        return data;
    }

    memcpy(tmp.buf, data.buf, data.len);
    U32 extraSpace = (U32)(minSize - data.len);
    memset(tmp.buf + data.len, ' ', extraSpace);

    return STRING_LEN(tmp.buf, data.len + extraSpace);
}

string stringWithMinSizeDefault(string data, U8 minSize) {
    return stringWithMinSize(data, minSize, stringConverterBuffer);
}

string noAppend() {
    ASSERT(false);
    return EMPTY_STRING;
}
