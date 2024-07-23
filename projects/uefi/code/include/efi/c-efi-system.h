#ifndef EFI_C_EFI_SYSTEM_H
#define EFI_C_EFI_SYSTEM_H

#pragma once

/**
 * UEFI System Integration
 *
 * This header defines the structures and types of the surrounding system of an
 * UEFI application. It contains the definitions of the system table, the
 * runtime and boot services, as well as common types.
 *
 * We do not document the behavior of each of these types and functions. They
 * follow the UEFI specification, which does a well-enough job of documenting
 * each. This file just provides you the C definitions of each symbol and some
 * limited hints on some pecularities.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "configuration-table.h"

/*
 * Time Management
 *
 * UEFI time management is modeled around the Time structure, which
 * represents any arbitrary timestamp. The runtime and boot services provide
 * helper functions to query and set the system time.
 */

#define C_EFI_TIME_ADJUST_DAYLIGHT U8_C(0x01)
#define C_EFI_TIME_IN_DAYLIGHT U8_C(0x02)

#define C_EFI_UNSPECIFIED_TIMEZONE I16_C(0x07ff)

typedef struct Time {
    U16 year;
    U8 month;
    U8 day;
    U8 hour;
    U8 minute;
    U8 second;
    U8 pad1;
    U32 nanosecond;
    I16 timezone;
    U8 daylight;
    U8 pad2;
} Time;

typedef struct TimeCapabilities {
    U32 resolution;
    U32 accuracy;
    bool sets_to_zero;
} TimeCapabilities;

/*
 * UEFI Variables
 *
 * UEFI systems provide a way to store global variables. These can be
 * persistent or volatile. The variable store must be provided by the platform,
 * but persistent storage might not be available.
 */

#define C_EFI_VARIABLE_NON_VOLATILE U32_C(0x00000001)
#define C_EFI_VARIABLE_BOOTSERVICE_ACCESS U32_C(0x00000002)
#define C_EFI_VARIABLE_RUNTIME_ACCESS U32_C(0x00000004)
#define C_EFI_VARIABLE_HARDWARE_ERROR_RECORD U32_C(0x00000008)
#define C_EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS U32_C(0x00000010)
#define C_EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS                   \
    U32_C(0x00000020)
#define C_EFI_VARIABLE_APPEND_WRITE U32_C(0x00000040)
#define C_EFI_VARIABLE_ENHANCED_AUTHENTICATED_ACCESS U32_C(0x00000080)

#define C_EFI_VARIABLE_AUTHENTICATION_3_CERT_ID_SHA256 U32_C(1)

typedef struct VariableAuthentication3CertId {
    U8 type;
    U32 id_size;
    U8 id[];
} VariableAuthentication3CertId;

typedef struct VariableAuthentication {
    U64 monotonic_count;
    U8 auth_info[]; /* WIN_CERTIFICATE_UEFI_ID from PE/COFF */
} VariableAuthentication;

typedef struct VariableAuthentication2 {
    Time timestamp;
    U8 auth_info[]; /* WIN_CERTIFICATE_UEFI_ID from PE/COFF */
} VariableAuthentication2;

#define C_EFI_VARIABLE_AUTHENTICATION_3_TIMESTAMP_TYPE U32_C(1)
#define C_EFI_VARIABLE_AUTHENTICATION_3_NONCE_TYPE U32_C(2)

typedef struct VariableAuthentication3 {
    U8 version;
    U8 type;
    U32 metadata_size;
    U32 flags;
} VariableAuthentication3;

typedef struct VariableAuthentication3Nonce {
    U32 nonce_size;
    U8 nonce[];
} VariableAuthentication3Nonce;

#define C_EFI_HARDWARE_ERROR_VARIABLE_GUID                                     \
    EFI_GUID(0x414E6BDD, 0xE47B, 0x47cc, 0xB2, 0x44, 0xBB, 0x61, 0x02, 0x0C, \
               0xF5, 0x16)

/*
 * Virtual Mappings
 *
 * UEFI runs in an 1-to-1 mapping from virtual to physical addresses. But once
 * you exit boot services, you can apply any address mapping you want, as long
 * as you inform UEFI about it (or, alternatively, stop using the UEFI runtime
 * services).
 */

#define C_EFI_OPTIONAL_POINTER U32_C(0x00000001)

/*
 * System Reset
 *
 * UEFI provides access to firmware functions to reset the system. This
 * includes a wide variety of different possible resets.
 */

typedef enum ResetType {
    C_EFI_RESET_COLD,
    C_EFI_RESET_WARM,
    C_EFI_RESET_SHUTDOWN,
    C_EFI_RESET_PLATFORM_SPECIFIC,
    C_EFI_RESET_N,
} ResetType;

/*
 * Update Capsules
 *
 * The process of firmware updates is generalized in UEFI. There are small
 * blobs called capsules that you can push into the firmware to be run either
 * immediately or on next reboot.
 */

typedef struct CapsuleBlockDescriptor {
    U64 length;
    union {
        PhysicalAddress data_block;
        PhysicalAddress continuation_pointer;
    };
} CapsuleBlockDescriptor;

#define C_EFI_CAPSULE_FLAGS_PERSIST_ACROSS_RESET C_EFI_U32(0x00010000)
#define C_EFI_CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE C_EFI_U32(0x00020000)
#define C_EFI_CAPSULE_FLAGS_INITIATE_RESET C_EFI_U32(0x00040000)

typedef struct CapsuleHeader {
    Guid capsule_guid;
    U32 header_size;
    U32 flags;
    U32 capsule_image_size;
} CapsuleHeader;

#define C_EFI_OS_INDICATIONS_BOOT_TO_FW_UI U64_C(0x0000000000000001)
#define C_EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION                              \
    U64_C(0x0000000000000002)
#define C_EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED                   \
    U64_C(0x0000000000000004)
#define C_EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED                             \
    U64_C(0x0000000000000008)
#define C_EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED                      \
    U64_C(0x0000000000000010)
#define C_EFI_OS_INDICATIONS_START_OS_RECOVERY U64_C(0x0000000000000020)
#define C_EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY                           \
    U64_C(0x0000000000000040)

#define C_EFI_CAPSULE_REPORT_GUID                                              \
    EFI_GUID(0x39b68c46, 0xf7fb, 0x441b, 0xb6, 0xec, 0x16, 0xb0, 0xf6, 0x98, \
               0x21, 0xf3)

typedef struct CapsuleResultVariableHeader {
    U32 variable_total_size;
    U32 reserved;
    Guid capsule_guid;
    Time capsule_processed;
    Status capsule_status;
} CapsuleResultVariableHeader;

typedef struct CapsuleResultVariableFMP {
    U16 version;
    U8 payload_index;
    U8 update_image_index;
    Guid update_image_type_id;
    U16 capsule_file_name_and_target[];
} CapsuleResultVariableFMP;

/*
 * Tasks
 *
 * UEFI uses a simplified task model, and only ever runs on a single CPU.
 * Usually, there is only one single task running on the system, which is the
 * current execution. No interrupts are supported, other than timer interrupts.
 * That is, all device management must be reliant on polling.
 *
 * You can, however, register callbacks to be run by the UEFI core. That is,
 * either when execution is returned to the UEFI core, or when a timer
 * interrupt fires, the scheduler will run the highest priority task next,
 * interrupting the current task. You can use simple task-priority-levels (TPL)
 * to adjust the priority of your callbacks and current task.
 */

#define C_EFI_EVT_TIMER U32_C(0x80000000)
#define C_EFI_EVT_RUNTIME U32_C(0x40000000)
#define C_EFI_EVT_NOTIFY_WAIT U32_C(0x00000100)
#define C_EFI_EVT_NOTIFY_SIGNAL U32_C(0x00000200)
#define C_EFI_EVT_SIGNAL_EXIT_BOOT_SERVICES U32_C(0x00000201)
#define C_EFI_EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE U32_C(0x60000202)

typedef void(CEFICALL *EventNotify)(Event event, void *context);

#define C_EFI_EVENT_GROUP_EXIT_BOOT_SERVICES                                   \
    EFI_GUID(0x27abf055, 0xb1b8, 0x4c26, 0x80, 0x48, 0x74, 0x8f, 0x37, 0xba, \
               0xa2, 0xdf)
#define C_EFI_EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE                               \
    EFI_GUID(0x13fa7698, 0xc831, 0x49c7, 0x87, 0xea, 0x8f, 0x43, 0xfc, 0xc2, \
               0x51, 0x96)
#define C_EFI_EVENT_GROUP_MEMORY_MAP_CHANGE                                    \
    EFI_GUID(0x78bee926, 0x692f, 0x48fd, 0x9e, 0xdb, 0x1, 0x42, 0x2e, 0xf0,  \
               0xd7, 0xab)
#define C_EFI_EVENT_GROUP_READY_TO_BOOT                                        \
    EFI_GUID(0x7ce88fb3, 0x4bd7, 0x4679, 0x87, 0xa8, 0xa8, 0xd8, 0xde, 0xe5, \
               0x0d, 0x2b)
#define C_EFI_EVENT_GROUP_RESET_SYSTEM                                         \
    EFI_GUID(0x62da6a56, 0x13fb, 0x485a, 0xa8, 0xda, 0xa3, 0xdd, 0x79, 0x12, \
               0xcb, 0x6b)

typedef enum TimerDelay {
    C_EFI_TIMER_CANCEL,
    C_EFI_TIMER_PERIODIC,
    C_EFI_TIMER_RELATIVE,
} TimerDelay;

#define C_EFI_TPL_APPLICATION 4
#define C_EFI_TPL_CALLBACK 8
#define C_EFI_TPL_NOTIFY 16
#define C_EFI_TPL_HIGH_LEVEL 31

/*
 * Memory management
 *
 * The UEFI boot services provide you pool-allocation helpers to reserve
 * memory. The region for each allocation can be selected by the caller,
 * allowing to reserve memory that even survives beyond boot services. However,
 * dynamic allocations can only performed via boot services, so no dynamic
 * modifications can be done once you exit boot services.
 */

typedef enum AllocateType {
    C_EFI_ALLOCATE_ANY_PAGES,
    C_EFI_ALLOCATE_MAX_ADDRESS,
    C_EFI_ALLOCATE_ADDRESS,
    C_EFI_ALLOCATE_TYPE_N,
} AllocateType;

typedef enum MemoryType {
    C_EFI_RESERVED_MEMORY_TYPE,
    C_EFI_LOADER_CODE,
    C_EFI_LOADER_DATA,
    C_EFI_BOOT_SERVICES_CODE,
    C_EFI_BOOT_SERVICES_DATA,
    C_EFI_RUNTIME_SERVICES_CODE,
    C_EFI_RUNTIME_SERVICES_DATA,
    C_EFI_CONVENTIONAL_MEMORY,
    C_EFI_UNUSABLE_MEMORY,
    C_EFI_ACPI_RECLAIM_MEMORY,
    C_EFI_ACPI_MEMORY_NVS,
    C_EFI_MEMORY_MAPPED_IO,
    C_EFI_MEMORY_MAPPED_IO_PORT_SPACE,
    C_EFI_PAL_CODE,
    C_EFI_PERSISTENT_MEMORY,
    C_EFI_MEMORY_TYPE_N,
} MemoryType;

#define C_EFI_MEMORY_UC U64_C(0x0000000000000001)
#define C_EFI_MEMORY_WC U64_C(0x0000000000000002)
#define C_EFI_MEMORY_WT U64_C(0x0000000000000004)
#define C_EFI_MEMORY_WB U64_C(0x0000000000000008)
#define C_EFI_MEMORY_UCE U64_C(0x0000000000000010)
#define C_EFI_MEMORY_WP U64_C(0x0000000000001000)
#define C_EFI_MEMORY_RP U64_C(0x0000000000002000)
#define C_EFI_MEMORY_XP U64_C(0x0000000000004000)
#define C_EFI_MEMORY_NV U64_C(0x0000000000008000)
#define C_EFI_MEMORY_MORE_RELIABLE U64_C(0x0000000000010000)
#define C_EFI_MEMORY_RO U64_C(0x0000000000020000)
#define C_EFI_MEMORY_RUNTIME U64_C(0x8000000000000000)

#define C_EFI_MEMORY_DESCRIPTOR_VERSION U32_C(0x00000001)

typedef struct MemoryDescriptor {
    U32 type;
    PhysicalAddress physical_start;
    VirtualAddress virtual_start;
    U64 number_of_pages;
    U64 attribute;
} MemoryDescriptor;

/*
 * Protocol Management
 *
 * The UEFI driver model provides ways to have bus-drivers, device-drivers, and
 * applications as separate, independent entities. They use protocols to
 * communicate, and handles to refer to common state. Drivers and devices can
 * be registered dynamically at runtime, and can support hotplugging.
 */

typedef enum InterfaceType {
    C_EFI_NATIVE_INTERFACE,
} InterfaceType;

typedef enum LocateSearchType {
    C_EFI_ALL_HANDLES,
    C_EFI_BY_REGISTER_NOTIFY,
    C_EFI_BY_PROTOCOL,
} LocateSearchType;

#define C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL U32_C(0x00000001)
#define C_EFI_OPEN_PROTOCOL_GET_PROTOCOL U32_C(0x00000002)
#define C_EFI_OPEN_PROTOCOL_TEST_PROTOCOL U32_C(0x00000004)
#define C_EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER U32_C(0x00000008)
#define C_EFI_OPEN_PROTOCOL_BY_DRIVER U32_C(0x00000010)
#define C_EFI_OPEN_PROTOCOL_EXCLUSIVE U32_C(0x00000020)

typedef struct OpenProtocolInformationEntry {
    Handle agent_handle;
    Handle controller_handle;
    U32 attributes;
    U32 open_count;
} OpenProtocolInformationEntry;

/*
 * Configuration Tables
 *
 * The system table contains an array of auxiliary tables, indexed by their
 * GUID, called configuration tables. Each table uses the generic
 * ConfigurationTable structure as header.
 */

#define C_EFI_SAL_SYSTEM_TABLE_GUID                                            \
    EFI_GUID(0xeb9d2d32, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, \
               0xc1, 0x4d)
#define C_EFI_SMBIOS_TABLE_GUID                                                \
    EFI_GUID(0xeb9d2d31, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, \
               0xc1, 0x4d)
#define C_EFI_SMBIOS3_TABLE_GUID                                               \
    EFI_GUID(0xf2fd1544, 0x9794, 0x4a2c, 0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, \
               0xe3, 0x94)
#define C_EFI_MPS_TABLE_GUID                                                   \
    EFI_GUID(0xeb9d2d2f, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, \
               0xc1, 0x4d)

#define C_EFI_PROPERTIES_TABLE_GUID                                            \
    EFI_GUID(0x880aaca3, 0x4adc, 0x4a04, 0x90, 0x79, 0xb7, 0x47, 0x34, 0x8,  \
               0x25, 0xe5)
#define C_EFI_PROPERTIES_TABLE_VERSION U32_C(0x00010000)

#define C_EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA      \
    U64_C(0x1)

typedef struct PropertiesTable {
    U32 version;
    U32 length;
    U64 memory_protection_attribute;
} PropertiesTable;

#define C_EFI_MEMORY_ATTRIBUTES_TABLE_GUID                                     \
    EFI_GUID(0xdcfa911d, 0x26eb, 0x469f, 0xa2, 0x20, 0x38, 0xb7, 0xdc, 0x46, \
               0x12, 0x20)
#define C_EFI_MEMORY_ATTRIBUTES_TABLE_VERSION U32_C(0x00000001)

typedef struct MemoryAttributesTable {
    U32 version;
    U32 number_of_entries;
    U32 descriptor_size;
    U32 reserved;
    MemoryDescriptor entry[];
} MemoryAttributesTable;

/*
 * Global Tables
 *
 * UEFI uses no global state, so all access to UEFI internal state is done
 * through vtables you get passed to your entry-point. The global entry is the
 * system-table, which encorporates several sub-tables, including the runtime
 * and boot service tables, and configuration tables (including vendor
 * extensions).
 */

#define C_EFI_2_70_SYSTEM_TABLE_REVISION ((2 << 16) | (70))
#define C_EFI_2_60_SYSTEM_TABLE_REVISION ((2 << 16) | (60))
#define C_EFI_2_50_SYSTEM_TABLE_REVISION ((2 << 16) | (50))
#define C_EFI_2_40_SYSTEM_TABLE_REVISION ((2 << 16) | (40))
#define C_EFI_2_31_SYSTEM_TABLE_REVISION ((2 << 16) | (31))
#define C_EFI_2_30_SYSTEM_TABLE_REVISION ((2 << 16) | (30))
#define C_EFI_2_20_SYSTEM_TABLE_REVISION ((2 << 16) | (20))
#define C_EFI_2_10_SYSTEM_TABLE_REVISION ((2 << 16) | (10))
#define C_EFI_2_00_SYSTEM_TABLE_REVISION ((2 << 16) | (0))
#define C_EFI_1_10_SYSTEM_TABLE_REVISION ((1 << 16) | (10))
#define C_EFI_1_02_SYSTEM_TABLE_REVISION ((1 << 16) | (2))
#define C_EFI_SPECIFICATION_VERSION C_EFI_SYSTEM_TABLE_REVISION
#define C_EFI_SYSTEM_TABLE_REVISION C_EFI_2_70_SYSTEM_TABLE_REVISION
#define C_EFI_RUNTIME_SERVICES_REVISION C_EFI_SPECIFICATION_VERSION
#define C_EFI_BOOT_SERVICES_REVISION C_EFI_SPECIFICATION_VERSION

typedef struct TableHeader {
    U64 signature;
    U32 revision;
    U32 header_size;
    U32 crc32;
    U32 reserved;
} TableHeader;

#define C_EFI_RUNTIME_TABLE_SIGNATURE                                          \
    U64_C(0x56524553544e5552) /* "RUNTSERV" */

typedef struct RuntimeServices {
    TableHeader hdr;

    Status(CEFICALL *get_time)(Time *time,
                                   TimeCapabilities *capabilities

    );
    Status(CEFICALL *set_time)(Time *time);
    Status(CEFICALL *get_wakeup_time)(bool *enabled, bool *pending,
                                          Time *time);
    Status(CEFICALL *set_wakeup_time)(bool enable, Time *time);

    Status(CEFICALL *set_virtual_address_map)(
        USize memory_map_size, USize descriptor_size,
        U32 descriptor_version, MemoryDescriptor *virtual_map);
    Status(CEFICALL *convert_pointer)(USize debug_disposition,
                                          void **address);

    Status(CEFICALL *get_variable)(U16 *variable_name,
                                       Guid *vendor_guid,
                                       U32 *attributes,
                                       USize *data_size, void *data);
    Status(CEFICALL *get_next_variable_name)(USize *variable_name_size,
                                                 U16 *variable_name,
                                                 Guid *vendor_guid);
    Status(CEFICALL *set_variable)(U16 *variable_name,
                                       Guid *vendor_guid,
                                       U32 attributes, USize data_size,
                                       void *data);

    Status(CEFICALL *get_next_high_mono_count)(U32 *high_count);
    void(CEFICALL *reset_system)(ResetType reset_type,
                                 Status reset_status, USize data_size,
                                 void *reset_data);

    Status(CEFICALL *update_capsule)(
        CapsuleHeader **capsule_header_array, USize capsule_count,
        PhysicalAddress scatter_gather_list);
    Status(CEFICALL *query_capsule_capabilities)(
        CapsuleHeader **capsule_header_array, USize capsule_count,
        U64 *maximum_capsule_size, ResetType *reset_type);

    Status(CEFICALL *query_variable_info)(
        U32 attributes, U64 *maximum_variable_storage_size,
        U64 *remaining_variable_storage_size,
        U64 *maximum_variable_size);
} RuntimeServices;

#define C_EFI_BOOT_SERVICES_SIGNATURE                                          \
    U64_C(0x56524553544f4f42) /* "BOOTSERV" */

typedef struct BootServices {
    TableHeader hdr;

    Tpl(CEFICALL *raise_tpl)(Tpl new_tpl);
    void(CEFICALL *restore_tpl)(Tpl old_tpl);

    Status(CEFICALL *allocate_pages)(AllocateType type,
                                         MemoryType memory_type,
                                         USize pages,
                                         PhysicalAddress *memory);
    Status(CEFICALL *free_pages)(PhysicalAddress memory,
                                     USize pages);
    Status(CEFICALL *get_memory_map)(USize *memory_map_size,
                                         MemoryDescriptor *memory_map,
                                         USize *map_key,
                                         USize *descriptor_size,
                                         U32 *descriptor_version);
    Status(CEFICALL *allocate_pool)(MemoryType pool_type,
                                        USize size, void **buffer);
    Status(CEFICALL *free_pool)(void *buffer);

    Status(CEFICALL *create_event)(U32 type, Tpl notify_tpl,
                                       EventNotify notify_function,
                                       void *notify_context, Event *event);
    Status(CEFICALL *set_timer)(Event event, TimerDelay type,
                                    U64 trigger_time);
    Status(CEFICALL *wait_for_event)(USize number_of_events,
                                         Event *event, USize *index);
    Status(CEFICALL *signal_event)(Event event);
    Status(CEFICALL *close_event)(Event event);
    Status(CEFICALL *check_event)(Event event);

    Status(CEFICALL *install_protocol_interface)(
        Handle *handle, Guid *protocol,
        InterfaceType interface_type, void *interface);
    Status(CEFICALL *reinstall_protocol_interface)(Handle handle,
                                                       Guid *protocol,
                                                       void *old_interface,
                                                       void *new_interface);
    Status(CEFICALL *uninstall_protocol_interface)(Handle handle,
                                                       Guid *protocol,
                                                       void *interface);
    Status(CEFICALL *handle_protocol)(Handle handle, Guid *protocol,
                                          void **interface);
    void *reserved;
    Status(CEFICALL *register_protocol_notify)(Guid *protocol,
                                                   Event event,
                                                   void **registration);
    Status(CEFICALL *locate_handle)(LocateSearchType search_type,
                                        Guid *protocol, void *search_key,
                                        USize *buffer_size,
                                        Handle *buffer);
    Status(CEFICALL *locate_device_path)(
        Guid *protocol, DevicePathProtocol **device_path,
        Handle *device);

    Status(CEFICALL *install_configuration_table)(Guid *guid,
                                                      void *table);

    Status(CEFICALL *load_image)(bool boot_policy,
                                     Handle parent_image_handle,
                                     DevicePathProtocol *device_path,
                                     void *source_buffer, USize source_size,
                                     Handle *image_handle);
    Status(CEFICALL *start_image)(Handle image_handle,
                                      USize *exit_data_size,
                                      U16 **exit_data);
    Status(CEFICALL *exit)(Handle image_handle, Status exit_status,
                               USize exit_data_size, U16 *exit_data);
    Status(CEFICALL *unload_image)(Handle image_handle);
    Status(CEFICALL *exit_boot_services)(Handle image_handle,
                                             USize map_key);

    Status(CEFICALL *get_next_monotonic_count)(U64 *count);
    Status(CEFICALL *stall)(USize microseconds);
    Status(CEFICALL *set_watchdog_timer)(USize timeout,
                                             U64 watchdog_code,
                                             USize data_size,
                                             U16 *watchdog_data);

    /* 1.1+ */

    Status(CEFICALL *connect_controller)(
        Handle controller_handle, Handle *driver_image_handle,
        DevicePathProtocol *remaining_device_path, bool recursive);
    Status(CEFICALL *disconnect_controller)(Handle controller_handle,
                                                Handle driver_image_handle,
                                                Handle child_handle);

    Status(CEFICALL *open_protocol)(Handle handle, Guid *protocol,
                                        void **interface,
                                        Handle agent_handle,
                                        Handle controller_handle,
                                        U32 attributes);
    Status(CEFICALL *close_protocol)(Handle handle, Guid *protocol,
                                         Handle agent_handle,
                                         Handle controller_handle);
    Status(CEFICALL *open_protocol_information)(
        Handle handle, Guid *protocol,
        OpenProtocolInformationEntry **entry_buffer,
        USize *entry_count);

    Status(CEFICALL *protocols_per_handle)(
        Handle handle, Guid ***protocol_buffer,
        USize *protocol_buffer_count);
    Status(CEFICALL *locate_handle_buffer)(LocateSearchType search_type,
                                               Guid *protocol,
                                               void *search_key,
                                               USize *no_handles,
                                               Handle **buffer);
    Status(CEFICALL *locate_protocol)(Guid *protocol,
                                          void *registration, void **interface);
    Status(CEFICALL *install_multiple_protocol_interfaces)(
        Handle *handle, ...);
    Status(CEFICALL *uninstall_multiple_protocol_interfaces)(
        Handle handle, ...);

    Status(CEFICALL *calculate_crc32)(void *data, USize data_size,
                                          U32 *crc32);

    void(CEFICALL *copy_mem)(void *destination, void *source, USize length);
    void(CEFICALL *set_mem)(void *buffer, USize size, U8 value);

    /* 2.0+ */

    Status(CEFICALL *create_event_ex)(U32 type, Tpl notify_tpl,
                                          EventNotify notify_function,
                                          void *notify_context,
                                          Guid *event_group,
                                          Event *event);
} BootServices;

#define C_EFI_SYSTEM_TABLE_SIGNATURE                                           \
    U64_C(0x5453595320494249) /* "IBI SYST" */

typedef struct SystemTable {
    TableHeader hdr;
    U16 *firmware_vendor;
    U32 firmware_revision;

    Handle console_in_handle;
    SimpleTextInputProtocol *con_in;
    Handle console_out_handle;
    SimpleTextOutputProtocol *con_out;
    Handle standard_error_handle;
    SimpleTextOutputProtocol *std_err;

    RuntimeServices *runtime_services;
    BootServices *boot_services;

    USize number_of_table_entries;
    ConfigurationTable *configuration_table;
} SystemTable;

#ifdef __cplusplus
}
#endif

#endif
