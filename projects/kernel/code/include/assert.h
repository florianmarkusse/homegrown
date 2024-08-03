#ifndef ASSERT_H
#define ASSERT_H

#define STATIC_ASSERT(COND, MSG)                                               \
    typedef char static_assertion_##MSG[(COND) ? 1 : -1]

#endif
