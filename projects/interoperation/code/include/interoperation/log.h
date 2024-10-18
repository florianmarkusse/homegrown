#ifndef INTEROPERATION_LOG_H
#define INTEROPERATION_LOG_H

static constexpr auto NEWLINE = 0x01;
static constexpr auto FLUSH = 0x02;

#ifdef FREESTANDING_BUILD

#define LOG_1(data) LOG_DATA(data, 0)
#define LOG_2(data, flags) LOG_DATA(data, flags)

#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define LOG_CHOOSER_IMPL(_1, _2, N, ...) LOG_CHOOSER_IMPL_##N

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define INFO(data, ...) LOG(data, ##__VA_ARGS__)

#define ERROR(data, ...) LOG(data, ##__VA_ARGS__)

#else

#define LOG_3(data, bufferType, flags)                                         \
    LOG_DATA_BUFFER_TYPE(data, bufferType, flags)
#define LOG_2(data, bufferType) LOG_DATA_BUFFER_TYPE(data, bufferType, 0)
#define LOG_1(data) LOG_DATA_BUFFER_TYPE(data, STDOUT, 0)

#define LOG_CHOOSER_IMPL_3(arg1, arg2, arg3) LOG_3(arg1, arg2, arg3)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL(_1, _2, _3, N, ...) LOG_CHOOSER_IMPL_##N
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 3, 2, 1)

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define INFO(data, ...) LOG(data, STDOUT, ##__VA_ARGS__)

#define ERROR(data, ...) LOG(data, STDERR, ##__VA_ARGS__)

#endif

#endif
