#ifndef SHARED_MEMORY_ALLOCATOR_MACROS_H
#define SHARED_MEMORY_ALLOCATOR_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

static constexpr auto ZERO_MEMORY = 0x01;
static constexpr auto nullptr_ON_FAIL = 0x02;

#ifdef __cplusplus
}
#endif

#endif
