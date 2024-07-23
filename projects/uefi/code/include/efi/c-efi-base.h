#ifndef EFI_C_EFI_BASE_H
#define EFI_C_EFI_BASE_H

#pragma once

#include "types.h"

/**
 * UEFI Base Environment
 *
 * This header defines the base environment for UEFI development. It provides
 * types and macros as declared in the UEFI specification, as well as de-facto
 * standard additions provided by the reference implementation by Intel.
 *
 * This file does not depend on a standard library, but can be used as base to
 * port a standard library to UEFI. Note, though, the ISO-C Standard Library is
 * in many ways incompatible to the style of UEFI development. While it is
 * technically possible to implement it, it would work against many of the UEFI
 * characteristics.
 *
 * This header provides the base types and macros used throughout the project.
 * It provides basic fixed-size integers, a NULL-equivalent, booleans, standard
 * UEFI types, and more. All symbols are prefixed with `C_EFI_*` or `*`.
 *
 * You are highly recommended to conduct the UEFI Specification for details on
 * the programming environment. Following a summary of key parts from the
 * specification:
 *
 *  * All integers are either fixed-size, or native size. That is, either use
 *    {8,..,64} and U{8,..,64} directly, or use the native-size
 *    Size and USize. Native size integers are sized according to the
 *    architecture restrictions. You should assume they are pointer-sized.
 *
 *    Whenever you refer to memory (either pointing to it, or remember the size
 *    of a memory block), the native size integers should be your tool of
 *    choice.
 *
 *  * Even though the CPU might run in any endianness, all stored data is
 *    little-endian. That means, if you encounter integers split into
 *    byte-arrays (e.g., `DevicePathProtocol.length`), you must assume it
 *    is little-endian encoded. But if you encounter native integers, you must
 *    assume they are encoded in native endianness.
 *    For now the UEFI specification only defines little-endian architectures,
 *    hence this did not pop up as actual issue. Future extensions might change
 *    this, though.
 *
 *  * The Microsoft calling-convention is used. If you configure your compiler
 *    correctly, you should be good to go. In all other cases, all UEFI
 *    functions are annotated with the correct calling-convention. As long as
 *    your compiler supports it, it will automatically pick the correct style.
 *    The UEFI Specification defines some additional common rules for all its
 *    APIs, though. You will most likely not see any of these mentioned in the
 *    individual API documentions, though. Here is a short reminder:
 *
 *     - Pointers must reference physical-memory locations (no I/O mappings, no
 *       virtual addresses, etc.). Once ExitBootServices() was called, and the
 *       virtual address mapping was set, you must provide virtual-memory
 *       locations instead.
 *     - Pointers must be correctly aligned.
 *     - NULL is disallowed, unless explicitly mentioned otherwise.
 *     - Data referenced by pointers is undefined on error-return from a
 *       function.
 *     - You must not pass data larger than native-size (sizeof(USize)) on
 *       the stack. You must pass them by reference.
 *
 *  * Stack size is at least 128KiB and 16-byte aligned. All stack space might
 *    be marked non-executable! Once ExitBootServices() was called, you must
 *    guarantee at least 4KiB of stack space, 16-byte aligned for all runtime
 *    services you call. These numbers differ depending on the target
 *    architecture, but should be roughly the same.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CEFICALL: Annotate Functions with UEFI Calling-Convention
 *
 * This macro annotates function declarations with the correct calling
 * convention. The UEFI Specification defines the calling-convention for each
 * architecture it supports in great detail. It is almost identical to the
 * calling-convention used on Microsoft Windows.
 */
#if defined(__arm__) || defined(_M_ARM)
#define CEFICALL __attribute__((pcs("aapcs")))
#elif defined(__aarch64__) || defined(_M_ARM64)
#define CEFICALL /* XXX: No ABI-specifier supported so far */
#elif defined(__i386__) || defined(_M_IX86)
#define CEFICALL __attribute__((cdecl))
#elif defined(__x86_64__) || defined(_M_X64)
#define CEFICALL __attribute__((ms_abi))
#else
#define CEFICALL /* Use native ABI; assume it matches the host. */
#endif

/*
 * The UEFI Specification has several circular type dependencies. We simply use
 * forward declarations to get the required types in-scope. We really try to
 * limit the number of circular type dependencies, so make sure to only add
 * them here if really necessary.
 */
typedef struct DevicePathProtocol DevicePathProtocol;
typedef struct SimpleTextInputProtocol SimpleTextInputProtocol;
typedef struct SimpleFileSystemProtocol SimpleFileSystemProtocol;
typedef struct DiskIOProtocol DiskIOProtocol;
typedef struct BlockIoProtocol BlockIoProtocol;
typedef struct FileProtocol FileProtocol;
typedef struct GraphicsOutputProtocol GraphicsOutputProtocol;
typedef struct SimpleTextOutputProtocol SimpleTextOutputProtocol;
typedef struct SystemTable SystemTable;
typedef struct ACPITableProtocol ACPITableProtocol;
typedef struct MPServicesProtocol MPServicesProtocol;

/**
 * Status: Status Codes
 *
 * The Status type is used to indicate the return status of functions,
 * operations, and internal state. A value of 0 indicates success. Positive
 * values (MSB unset) indicate warnings, negative values (MSB set) indicate
 * errors. The second-MSB distinguishes OEM warnings and errors.
 */
typedef USize Status;

#if __UINTPTR_MAX__ == __UINT32_MAX__
#define C_EFI_STATUS_C U32_C
#define C_EFI_STATUS_WIDTH 32
#elif __UINTPTR_MAX__ == __UINT64_MAX__
#define C_EFI_STATUS_C U64_C
#define C_EFI_STATUS_WIDTH 64
#else
#error "Unsupported value of __UINTPTR_MAX__"
#endif

#define C_EFI_STATUS_ERROR_MASK                                                \
    (C_EFI_STATUS_C(0x80) << (C_EFI_STATUS_WIDTH - 8))
#define C_EFI_STATUS_ERROR_OEM_MASK                                            \
    (C_EFI_STATUS_C(0xc0) << (C_EFI_STATUS_WIDTH - 8))
#define C_EFI_STATUS_WARNING_MASK                                              \
    (C_EFI_STATUS_C(0x00) << (C_EFI_STATUS_WIDTH - 8))
#define C_EFI_STATUS_WARNING_OEM_MASK                                          \
    (C_EFI_STATUS_C(0x40) << (C_EFI_STATUS_WIDTH - 8))

#define C_EFI_STATUS_ERROR_C(_x) (C_EFI_STATUS_C(_x) | C_EFI_STATUS_ERROR_MASK)
#define C_EFI_STATUS_ERROR_OEM_C(_x)                                           \
    (C_EFI_STATUS_C(_x) | C_EFI_STATUS_ERROR_OEM_MASK)
#define C_EFI_STATUS_WARNING_C(_x)                                             \
    (C_EFI_STATUS_C(_x) | C_EFI_STATUS_WARNING_MASK)
#define C_EFI_STATUS_WARNING_OEM_C(_x)                                         \
    (C_EFI_STATUS_C(_x) | C_EFI_STATUS_WARNING_OEM_MASK)

#define C_EFI_ERROR(_x) (!!((_x) & C_EFI_STATUS_ERROR_MASK))

#define C_EFI_SUCCESS C_EFI_STATUS_C(0)

#define C_EFI_LOAD_ERROR C_EFI_STATUS_ERROR_C(1)
#define C_EFI_INVALID_PARAMETER C_EFI_STATUS_ERROR_C(2)
#define C_EFI_UNSUPPORTED C_EFI_STATUS_ERROR_C(3)
#define C_EFI_BAD_BUFFER_SIZE C_EFI_STATUS_ERROR_C(4)
#define C_EFI_BUFFER_TOO_SMALL C_EFI_STATUS_ERROR_C(5)
#define C_EFI_NOT_READY C_EFI_STATUS_ERROR_C(6)
#define C_EFI_DEVICE_ERROR C_EFI_STATUS_ERROR_C(7)
#define C_EFI_WRITE_PROTECTED C_EFI_STATUS_ERROR_C(8)
#define C_EFI_OUT_OF_RESOURCES C_EFI_STATUS_ERROR_C(9)
#define C_EFI_VOLUME_CORRUPTED C_EFI_STATUS_ERROR_C(10)
#define C_EFI_VOLUME_FULL C_EFI_STATUS_ERROR_C(11)
#define C_EFI_NO_MEDIA C_EFI_STATUS_ERROR_C(12)
#define C_EFI_MEDIA_CHANGED C_EFI_STATUS_ERROR_C(13)
#define C_EFI_NOT_FOUND C_EFI_STATUS_ERROR_C(14)
#define C_EFI_ACCESS_DENIED C_EFI_STATUS_ERROR_C(15)
#define C_EFI_NO_RESPONSE C_EFI_STATUS_ERROR_C(16)
#define C_EFI_NO_MAPPING C_EFI_STATUS_ERROR_C(17)
#define C_EFI_TIMEOUT C_EFI_STATUS_ERROR_C(18)
#define C_EFI_NOT_STARTED C_EFI_STATUS_ERROR_C(19)
#define C_EFI_ALREADY_STARTED C_EFI_STATUS_ERROR_C(20)
#define C_EFI_ABORTED C_EFI_STATUS_ERROR_C(21)
#define C_EFI_ICMP_ERROR C_EFI_STATUS_ERROR_C(22)
#define C_EFI_TFTP_ERROR C_EFI_STATUS_ERROR_C(23)
#define C_EFI_PROTOCOL_ERROR C_EFI_STATUS_ERROR_C(24)
#define C_EFI_INCOMPATIBLE_VERSION C_EFI_STATUS_ERROR_C(25)
#define C_EFI_SECURITY_VIOLATION C_EFI_STATUS_ERROR_C(26)
#define C_EFI_CRC_ERROR C_EFI_STATUS_ERROR_C(27)
#define C_EFI_END_OF_MEDIA C_EFI_STATUS_ERROR_C(28)
#define C_EFI_END_OF_FILE C_EFI_STATUS_ERROR_C(31)
#define C_EFI_INVALID_LANGUAGE C_EFI_STATUS_ERROR_C(32)
#define C_EFI_COMPROMISED_DATA C_EFI_STATUS_ERROR_C(33)
#define C_EFI_IP_ADDRESS_CONFLICT C_EFI_STATUS_ERROR_C(34)
#define C_EFI_HTTP_ERROR C_EFI_STATUS_ERROR_C(35)

#define C_EFI_WARN_UNKNOWN_GLYPH C_EFI_STATUS_WARNING_C(1)
#define C_EFI_WARN_DELETE_FAILURE C_EFI_STATUS_WARNING_C(2)
#define C_EFI_WARN_WRITE_FAILURE C_EFI_STATUS_WARNING_C(3)
#define C_EFI_WARN_BUFFER_TOO_SMALL C_EFI_STATUS_WARNING_C(4)
#define C_EFI_WARN_STALE_DATA C_EFI_STATUS_WARNING_C(5)
#define C_EFI_WARN_FILE_SYSTEM C_EFI_STATUS_WARNING_C(6)
#define C_EFI_WARN_RESET_REQUIRED C_EFI_STATUS_WARNING_C(7)

/**
 * Handle, Event, Lba, Tpl, PhysicalAddress,
 * VirtualAddress: Common UEFI Aliases
 *
 * These types are all aliases as defined by the UEFI specification. They are
 * solely meant for documentational purposes.
 *
 * Handle represents handles to allocated objects. Event represents
 * slots that can be waited on (like Windows events). Lba represents
 * logical block addresses. Tpl represents thread priority levels.
 * PhysicalAddress, and VirtualAddress are used to denote physical,
 * and virtual addresses.
 */
typedef void *Handle;
typedef void *Event;
typedef U64 Lba;
typedef USize Tpl;
typedef U64 PhysicalAddress;
typedef U64 VirtualAddress;

/**
 * ImageEntryPoint: Type of image entry points
 *
 * All loaded images must have an entry point of this type. The entry point is
 * pointed to in the PE/COFF header. No particular symbol-name is required,
 * though most setups automatically pick the function named `efi_main`.
 *
 * On load, the entry-point is called with a pointer to the own image as first
 * argument, a pointer to the global system table as second argument. Normal
 * applications are unloaded when this function returns. Drivers might stay in
 * memory, depending on the return type. See the specification for details.
 */
typedef Status(CEFICALL *ImageEntryPoint)(Handle image, SystemTable *st);

/**
 * MacAddress, Ipv4Address,
 * Ipv6Address, IpAddress: Networking Types
 *
 * These types represent the corresponding networking entities. MacAddress,
 * Ipv4Address, and Ipv6Address are mere byte-buffers. IpAddress is
 * a 16-byte buffer, but required to be 4-byte aligned.
 */
typedef struct MacAddress {
    U8 u8[32];
} MacAddress;

typedef struct Ipv4Address {
    U8 u8[4];
} Ipv4Address;

typedef struct Ipv6Address {
    U8 u8[16];
} Ipv6Address;

typedef struct IpAddress {
    union {
        _Alignas(4) Ipv4Address ipv4;
        _Alignas(4) Ipv6Address ipv6;
    };
} IpAddress;

#ifdef __cplusplus
}
#endif

#endif
