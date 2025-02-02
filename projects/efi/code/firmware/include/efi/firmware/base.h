#ifndef EFI_FIRMWARE_BASE_H
#define EFI_FIRMWARE_BASE_H

#pragma once

#include "shared/types/types.h"

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
 * I8acteristics.
 *
 * This header provides the base types and macros used throughout the project.
 * It provides basic fixed-size integers, a nullptr-equivalent, booleans,
 * standard UEFI types, and more. All symbols are prefixed with `*` or `*`.
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
 *     - nullptr is disallowed, unless explicitly mentioned otherwise.
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

/**
 * EFICALL: Annotate Functions with UEFI Calling-Convention
 *
 * This macro annotates function declarations with the correct calling
 * convention. The UEFI Specification defines the calling-convention for each
 * architecture it supports in great detail. It is almost identical to the
 * calling-convention used on Microsoft Windows.
 */
#if defined(__arm__) || defined(_M_ARM)
#define EFICALL __attribute__((pcs("aapcs")))
#elif defined(__aarch64__) || defined(_M_ARM64)
#define EFICALL /* XXX: No ABI-specifier supported so far */
#elif defined(__i386__) || defined(_M_IX86)
#define EFICALL __attribute__((cdecl))
#elif defined(__x86_64__) || defined(_M_X64)
#define EFICALL __attribute__((ms_abi))
#else
#define EFICALL /* Use native ABI; assume it matches the host. */
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
#define STATUS_C U32_C
static constexpr auto STATUS_WIDTH = 32;
#elif __UINTPTR_MAX__ == __UINT64_MAX__
#define STATUS_C U64_C
static constexpr auto STATUS_WIDTH = 64;
#else
#error "Unsupported value of __UINTPTR_MAX__"
#endif

static constexpr auto STATUS_ERROR_MASK =
    (STATUS_C(0x80) << (STATUS_WIDTH - 8));
static constexpr auto STATUS_ERROR_OEM_MASK =
    (STATUS_C(0xc0) << (STATUS_WIDTH - 8));
static constexpr auto STATUS_WARNING_MASK =
    (STATUS_C(0x00) << (STATUS_WIDTH - 8));
static constexpr auto STATUS_WARNING_OEM_MASK =
    (STATUS_C(0x40) << (STATUS_WIDTH - 8));

#define STATUS_ERROR_C(_x) (STATUS_C(_x) | STATUS_ERROR_MASK)
#define STATUS_ERROR_OEM_C(_x) (STATUS_C(_x) | STATUS_ERROR_OEM_MASK)
#define STATUS_WARNING_C(_x) (STATUS_C(_x) | STATUS_WARNING_MASK)
#define STATUS_WARNING_OEM_C(_x) (STATUS_C(_x) | STATUS_WARNING_OEM_MASK)

#define EFI_ERROR(_x) (!!((_x) & STATUS_ERROR_MASK))

static constexpr auto SUCCESS = STATUS_C(0);

static constexpr auto LOAD_ERROR = STATUS_ERROR_C(1);
static constexpr auto INVALID_PARAMETER = STATUS_ERROR_C(2);
static constexpr auto UNSUPPORTED = STATUS_ERROR_C(3);
static constexpr auto BAD_BUFFER_SIZE = STATUS_ERROR_C(4);
static constexpr auto BUFFER_TOO_SMALL = STATUS_ERROR_C(5);
static constexpr auto NOT_READY = STATUS_ERROR_C(6);
static constexpr auto DEVICE_ERROR = STATUS_ERROR_C(7);
static constexpr auto WRITE_PROTECTED = STATUS_ERROR_C(8);
static constexpr auto OUT_OF_RESOURCES = STATUS_ERROR_C(9);
static constexpr auto VOLUME_CORRUPTED = STATUS_ERROR_C(10);
static constexpr auto VOLUME_FULL = STATUS_ERROR_C(11);
static constexpr auto NO_MEDIA = STATUS_ERROR_C(12);
static constexpr auto MEDIA_CHANGED = STATUS_ERROR_C(13);
static constexpr auto NOT_FOUND = STATUS_ERROR_C(14);
static constexpr auto ACCESS_DENIED = STATUS_ERROR_C(15);
static constexpr auto NO_RESPONSE = STATUS_ERROR_C(16);
static constexpr auto NO_MAPPING = STATUS_ERROR_C(17);
static constexpr auto TIMEOUT = STATUS_ERROR_C(18);
static constexpr auto NOT_STARTED = STATUS_ERROR_C(19);
static constexpr auto ALREADY_STARTED = STATUS_ERROR_C(20);
static constexpr auto ABORTED = STATUS_ERROR_C(21);
static constexpr auto ICMP_ERROR = STATUS_ERROR_C(22);
static constexpr auto TFTP_ERROR = STATUS_ERROR_C(23);
static constexpr auto PROTOCOL_ERROR = STATUS_ERROR_C(24);
static constexpr auto INCOMPATIBLE_VERSION = STATUS_ERROR_C(25);
static constexpr auto SECURITY_VIOLATION = STATUS_ERROR_C(26);
static constexpr auto CRC_ERROR = STATUS_ERROR_C(27);
static constexpr auto END_OF_MEDIA = STATUS_ERROR_C(28);
static constexpr auto END_OF_FILE = STATUS_ERROR_C(31);
static constexpr auto INVALID_LANGUAGE = STATUS_ERROR_C(32);
static constexpr auto COMPROMISED_DATA = STATUS_ERROR_C(33);
static constexpr auto IP_ADDRESS_CONFLICT = STATUS_ERROR_C(34);
static constexpr auto HTTP_ERROR = STATUS_ERROR_C(35);

static constexpr auto WARN_UNKNOWN_GLYPH = STATUS_WARNING_C(1);
static constexpr auto WARN_DELETE_FAILURE = STATUS_WARNING_C(2);
static constexpr auto WARN_WRITE_FAILURE = STATUS_WARNING_C(3);
static constexpr auto WARN_BUFFER_TOO_SMALL = STATUS_WARNING_C(4);
static constexpr auto WARN_STALE_DATA = STATUS_WARNING_C(5);
static constexpr auto WARN_FILE_SYSTEM = STATUS_WARNING_C(6);
static constexpr auto WARN_RESET_REQUIRED = STATUS_WARNING_C(7);

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
typedef Status(EFICALL *ImageEntryPoint)(Handle image, SystemTable *st);

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

#endif
