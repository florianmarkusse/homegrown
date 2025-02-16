#ifndef OS_LOADER_APIC_H
#define OS_LOADER_APIC_H

static constexpr auto APIC_LOCAL_ID_REGISTER = 0xFEE00020;
static constexpr auto APIC_SIV_REGISTER = 0xFEE000F0;
static constexpr auto APIC_ERROR_STATUS_REGISTER = 0xFEE00280;
static constexpr auto APIC_IPI_ICR_LOW = 0xFEE00300;
static constexpr auto APIC_IPI_ICR_HIGH = 0xFEE0310;
static constexpr auto APIC_LVT3_ERROR_REGISTER = 0xFEE0370;

static constexpr auto APIC_IPI_ICR_LOW_RESERVED = 0xFFF32000;
static constexpr auto APIC_IPI_ICR_HIGH_RESERVED = 0x00FFFFFF;

static constexpr auto APIC_SIV_ENABLE_LOCAL = (1 << 7);

static constexpr auto APIC_INIT_IPI = 0x00C4500;
static constexpr auto APIC_START_UP_IPI = 0x00C4600;

// TODO: this.
/* static constexpr auto APIC_ICR_DELIVERY_MODE()*/
static constexpr auto APIC_ICR_DELIVERY_STATUS = (1 << 12);

// Care must be taken that the value is correct based on the:
// - Destination Mode
// - Local Destination Register
// - Destination Format Register
#define APIC_IPI_ICR_SET_HIGH(destination)                                     \
    *((volatile U32 *)(APIC_IPI_ICR_HIGH)) =                                   \
        (*((volatile U32 *)(APIC_IPI_ICR_HIGH)) &                              \
         APIC_IPI_ICR_HIGH_RESERVED) |                                         \
        ((destination) << 24);

#define APIC_IPI_ICR_SET_LOW(bottom3Bytes)                                     \
    *((volatile U32 *)(APIC_IPI_ICR_LOW)) =                                    \
        (*((volatile U32 *)(APIC_IPI_ICR_LOW)) &                               \
         (APIC_IPI_ICR_LOW_RESERVED)) |                                        \
        (bottom3Bytes);

// #define send_ipi(destination, bottom3Bytes)                                    \
//     do {                                                                       \
//         while (*((volatile U32 *)(APIC_IPI_ICR_LOW)) &                     \
//                (APIC_ICR_DELIVERY_STATUS)) {                                   \
//             asm volatile("pause" : : : "memory");                      \
//         }                                                                      \
//         /* Setting high is not necessary when we are setting    */             \
//         /* a desination shorthand */                                           \
//         APIC_IPI_ICR_SET_HIGH(destination)                                     \
//         APIC_IPI_ICR_SET_LOW(bottom3Bytes)                                     \
//     } while (0)

#endif
