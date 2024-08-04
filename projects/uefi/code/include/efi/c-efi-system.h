#ifndef EFI_SYSTEM_H
#define EFI_SYSTEM_H

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
 * limited hints on some peculiarities.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "interoperation/configuration-table.h"
#include "interoperation/memory/descriptor.h"

/*
 * Time Management
 *
 * UEFI time management is modeled around the Time structure, which
 * represents any arbitrary timestamp. The runtime and boot services provide
 * helper functions to query and set the system time.
 */

#define TIME_ADJUST_DAYLIGHT U8_C(0x01)
#define TIME_IN_DAYLIGHT U8_C(0x02)

#define UNSPECIFIED_TIMEZONE I16_C(0x07ff)

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

#define VARIABLE_NON_VOLATILE U32_C(0x00000001)
#define VARIABLE_BOOTSERVICE_ACCESS U32_C(0x00000002)
#define VARIABLE_RUNTIME_ACCESS U32_C(0x00000004)
#define VARIABLE_HARDWARE_ERROR_RECORD U32_C(0x00000008)
#define VARIABLE_AUTHENTICATED_WRITE_ACCESS U32_C(0x00000010)
#define VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS U32_C(0x00000020)
#define VARIABLE_APPEND_WRITE U32_C(0x00000040)
#define VARIABLE_ENHANCED_AUTHENTICATED_ACCESS U32_C(0x00000080)

#define VARIABLE_AUTHENTICATION_3_CERT_ID_SHA256 U32_C(1)

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

#define VARIABLE_AUTHENTICATION_3_TIMESTAMP_TYPE U32_C(1)
#define VARIABLE_AUTHENTICATION_3_NONCE_TYPE U32_C(2)

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

#define HARDWARE_ERROR_VARIABLE_GUID                                           \
    EFI_GUID(0x414E6BDD, 0xE47B, 0x47cc, 0xB2, 0x44, 0xBB, 0x61, 0x02, 0x0C,   \
             0xF5, 0x16)

/*
 * Virtual Mappings
 *
 * UEFI runs in an 1-to-1 mapping from virtual to physical addresses. But once
 * you exit boot services, you can apply any address mapping you want, as long
 * as you inform UEFI about it (or, alternatively, stop using the UEFI runtime
 * services).
 */

#define OPTIONAL_POINTER U32_C(0x00000001)

/*
 * System Reset
 *
 * UEFI provides access to firmware functions to reset the system. This
 * includes a wide variety of different possible resets.
 */

typedef enum ResetType {
    RESET_COLD,
    RESET_WARM,
    RESET_SHUTDOWN,
    RESET_PLATFORM_SPECIFIC,
    RESET_N,
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

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET U32(0x00010000)
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE U32(0x00020000)
#define CAPSULE_FLAGS_INITIATE_RESET U32(0x00040000)

typedef struct CapsuleHeader {
    Guid capsule_guid;
    U32 header_size;
    U32 flags;
    U32 capsule_image_size;
} CapsuleHeader;

#define OS_INDICATIONS_BOOT_TO_FW_UI U64_C(0x0000000000000001)
#define OS_INDICATIONS_TIMESTAMP_REVOCATION U64_C(0x0000000000000002)
#define OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED U64_C(0x0000000000000004)
#define OS_INDICATIONS_FMP_CAPSULE_SUPPORTED U64_C(0x0000000000000008)
#define OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED U64_C(0x0000000000000010)
#define OS_INDICATIONS_START_OS_RECOVERY U64_C(0x0000000000000020)
#define OS_INDICATIONS_START_PLATFORM_RECOVERY U64_C(0x0000000000000040)

#define CAPSULE_REPORT_GUID                                                    \
    EFI_GUID(0x39b68c46, 0xf7fb, 0x441b, 0xb6, 0xec, 0x16, 0xb0, 0xf6, 0x98,   \
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

#define EVT_TIMER U32_C(0x80000000)
#define EVT_RUNTIME U32_C(0x40000000)
#define EVT_NOTIFY_WAIT U32_C(0x00000100)
#define EVT_NOTIFY_SIGNAL U32_C(0x00000200)
#define EVT_SIGNAL_EXIT_BOOT_SERVICES U32_C(0x00000201)
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE U32_C(0x60000202)

typedef void(EFICALL *EventNotify)(Event event, void *context);

#define EVENT_GROUP_EXIT_BOOT_SERVICES                                         \
    EFI_GUID(0x27abf055, 0xb1b8, 0x4c26, 0x80, 0x48, 0x74, 0x8f, 0x37, 0xba,   \
             0xa2, 0xdf)
#define EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE                                     \
    EFI_GUID(0x13fa7698, 0xc831, 0x49c7, 0x87, 0xea, 0x8f, 0x43, 0xfc, 0xc2,   \
             0x51, 0x96)
#define EVENT_GROUP_MEMORY_MAP_CHANGE                                          \
    EFI_GUID(0x78bee926, 0x692f, 0x48fd, 0x9e, 0xdb, 0x1, 0x42, 0x2e, 0xf0,    \
             0xd7, 0xab)
#define EVENT_GROUP_READY_TO_BOOT                                              \
    EFI_GUID(0x7ce88fb3, 0x4bd7, 0x4679, 0x87, 0xa8, 0xa8, 0xd8, 0xde, 0xe5,   \
             0x0d, 0x2b)
#define EVENT_GROUP_RESET_SYSTEM                                               \
    EFI_GUID(0x62da6a56, 0x13fb, 0x485a, 0xa8, 0xda, 0xa3, 0xdd, 0x79, 0x12,   \
             0xcb, 0x6b)

typedef enum TimerDelay {
    TIMER_CANCEL,
    TIMER_PERIODIC,
    TIMER_RELATIVE,
} TimerDelay;

#define TPL_APPLICATION 4
#define TPL_CALLBACK 8
#define TPL_NOTIFY 16
#define TPL_HIGH_LEVEL 31

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
    ALLOCATE_ANY_PAGES,
    ALLOCATE_MAX_ADDRESS,
    ALLOCATE_ADDRESS,
    ALLOCATE_TYPE_N,
} AllocateType;

/*
 * Protocol Management
 *
 * The UEFI driver model provides ways to have bus-drivers, device-drivers, and
 * applications as separate, independent entities. They use protocols to
 * communicate, and handles to refer to common state. Drivers and devices can
 * be registered dynamically at runtime, and can support hotplugging.
 */

typedef enum InterfaceType {
    NATIVE_INTERFACE,
} InterfaceType;

typedef enum LocateSearchType {
    ALL_HANDLES,
    BY_REGISTER_NOTIFY,
    BY_PROTOCOL,
} LocateSearchType;

#define OPEN_PROTOCOL_BY_HANDLE_PROTOCOL U32_C(0x00000001)
#define OPEN_PROTOCOL_GET_PROTOCOL U32_C(0x00000002)
#define OPEN_PROTOCOL_TEST_PROTOCOL U32_C(0x00000004)
#define OPEN_PROTOCOL_BY_CHILD_CONTROLLER U32_C(0x00000008)
#define OPEN_PROTOCOL_BY_DRIVER U32_C(0x00000010)
#define OPEN_PROTOCOL_EXCLUSIVE U32_C(0x00000020)

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

#define SAL_SYSTEM_TABLE_GUID                                                  \
    EFI_GUID(0xeb9d2d32, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f,   \
             0xc1, 0x4d)
#define SMBIOS_TABLE_GUID                                                      \
    EFI_GUID(0xeb9d2d31, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f,   \
             0xc1, 0x4d)
#define SMBIOS3_TABLE_GUID                                                     \
    EFI_GUID(0xf2fd1544, 0x9794, 0x4a2c, 0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20,   \
             0xe3, 0x94)
#define MPS_TABLE_GUID                                                         \
    EFI_GUID(0xeb9d2d2f, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f,   \
             0xc1, 0x4d)

#define PROPERTIES_TABLE_GUID                                                  \
    EFI_GUID(0x880aaca3, 0x4adc, 0x4a04, 0x90, 0x79, 0xb7, 0x47, 0x34, 0x8,    \
             0x25, 0xe5)
#define PROPERTIES_TABLE_VERSION U32_C(0x00010000)

#define PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA U64_C(0x1)

typedef struct PropertiesTable {
    U32 version;
    U32 length;
    U64 memory_protection_attribute;
} PropertiesTable;

#define MEMORY_ATTRIBUTES_TABLE_GUID                                           \
    EFI_GUID(0xdcfa911d, 0x26eb, 0x469f, 0xa2, 0x20, 0x38, 0xb7, 0xdc, 0x46,   \
             0x12, 0x20)
#define MEMORY_ATTRIBUTES_TABLE_VERSION U32_C(0x00000001)

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

#define EFI_2_70_SYSTEM_TABLE_REVISION ((2 << 16) | (70))
#define EFI_2_60_SYSTEM_TABLE_REVISION ((2 << 16) | (60))
#define EFI_2_50_SYSTEM_TABLE_REVISION ((2 << 16) | (50))
#define EFI_2_40_SYSTEM_TABLE_REVISION ((2 << 16) | (40))
#define EFI_2_31_SYSTEM_TABLE_REVISION ((2 << 16) | (31))
#define EFI_2_30_SYSTEM_TABLE_REVISION ((2 << 16) | (30))
#define EFI_2_20_SYSTEM_TABLE_REVISION ((2 << 16) | (20))
#define EFI_2_10_SYSTEM_TABLE_REVISION ((2 << 16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION ((2 << 16) | (0))
#define EFI_1_10_SYSTEM_TABLE_REVISION ((1 << 16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION ((1 << 16) | (2))
#define SPECIFICATION_VERSION SYSTEM_TABLE_REVISION
#define SYSTEM_TABLE_REVISION EFI_2_70_SYSTEM_TABLE_REVISION
#define RUNTIME_SERVICES_REVISION SPECIFICATION_VERSION
#define BOOT_SERVICES_REVISION SPECIFICATION_VERSION

typedef struct TableHeader {
    U64 signature;
    U32 revision;
    U32 header_size;
    U32 crc32;
    U32 reserved;
} TableHeader;

#define RUNTIME_TABLE_SIGNATURE U64_C(0x56524553544e5552) /* "RUNTSERV" */

typedef struct RuntimeServices {
    TableHeader hdr;

    Status(EFICALL *get_time)(Time *time, TimeCapabilities *capabilities

    );
    Status(EFICALL *set_time)(Time *time);
    Status(EFICALL *get_wakeup_time)(bool *enabled, bool *pending, Time *time);
    Status(EFICALL *set_wakeup_time)(bool enable, Time *time);

    Status(EFICALL *set_virtual_address_map)(USize memory_map_size,
                                             USize descriptor_size,
                                             U32 descriptor_version,
                                             MemoryDescriptor *virtual_map);
    Status(EFICALL *convert_pointer)(USize debug_disposition, void **address);

    Status(EFICALL *get_variable)(U16 *variable_name, Guid *vendor_guid,
                                  U32 *attributes, USize *data_size,
                                  void *data);
    Status(EFICALL *get_next_variable_name)(USize *variable_name_size,
                                            U16 *variable_name,
                                            Guid *vendor_guid);
    Status(EFICALL *set_variable)(U16 *variable_name, Guid *vendor_guid,
                                  U32 attributes, USize data_size, void *data);

    Status(EFICALL *get_next_high_mono_count)(U32 *high_count);
    void(EFICALL *reset_system)(ResetType reset_type, Status reset_status,
                                USize data_size, void *reset_data);

    Status(EFICALL *update_capsule)(CapsuleHeader **capsule_header_array,
                                    USize capsule_count,
                                    PhysicalAddress scatter_gather_list);
    Status(EFICALL *query_capsule_capabilities)(
        CapsuleHeader **capsule_header_array, USize capsule_count,
        U64 *maximum_capsule_size, ResetType *reset_type);

    Status(EFICALL *query_variable_info)(U32 attributes,
                                         U64 *maximum_variable_storage_size,
                                         U64 *remaining_variable_storage_size,
                                         U64 *maximum_variable_size);
} RuntimeServices;

#define BOOT_SERVICES_SIGNATURE U64_C(0x56524553544f4f42) /* "BOOTSERV" */

typedef struct BootServices {
    TableHeader hdr;

    Tpl(EFICALL *raise_tpl)(Tpl new_tpl);
    void(EFICALL *restore_tpl)(Tpl old_tpl);

    Status(EFICALL *allocate_pages)(AllocateType type, MemoryType memory_type,
                                    USize pages, PhysicalAddress *memory);
    Status(EFICALL *free_pages)(PhysicalAddress memory, USize pages);
    Status(EFICALL *get_memory_map)(USize *memory_map_size,
                                    MemoryDescriptor *memory_map,
                                    USize *map_key, USize *descriptor_size,
                                    U32 *descriptor_version);
    Status(EFICALL *allocate_pool)(MemoryType pool_type, USize size,
                                   void **buffer);
    Status(EFICALL *free_pool)(void *buffer);

    Status(EFICALL *create_event)(U32 type, Tpl notify_tpl,
                                  EventNotify notify_function,
                                  void *notify_context, Event *event);
    Status(EFICALL *set_timer)(Event event, TimerDelay type, U64 trigger_time);
    Status(EFICALL *wait_for_event)(USize number_of_events, Event *event,
                                    USize *index);
    Status(EFICALL *signal_event)(Event event);
    Status(EFICALL *close_event)(Event event);
    Status(EFICALL *check_event)(Event event);

    Status(EFICALL *install_protocol_interface)(Handle *handle, Guid *protocol,
                                                InterfaceType interface_type,
                                                void *interface);
    Status(EFICALL *reinstall_protocol_interface)(Handle handle, Guid *protocol,
                                                  void *old_interface,
                                                  void *new_interface);
    Status(EFICALL *uninstall_protocol_interface)(Handle handle, Guid *protocol,
                                                  void *interface);
    Status(EFICALL *handle_protocol)(Handle handle, Guid *protocol,
                                     void **interface);
    void *reserved;
    Status(EFICALL *register_protocol_notify)(Guid *protocol, Event event,
                                              void **registration);
    Status(EFICALL *locate_handle)(LocateSearchType search_type, Guid *protocol,
                                   void *search_key, USize *buffer_size,
                                   Handle *buffer);
    Status(EFICALL *locate_device_path)(Guid *protocol,
                                        DevicePathProtocol **device_path,
                                        Handle *device);

    Status(EFICALL *install_configuration_table)(Guid *guid, void *table);

    Status(EFICALL *load_image)(bool boot_policy, Handle parent_image_handle,
                                DevicePathProtocol *device_path,
                                void *source_buffer, USize source_size,
                                Handle *image_handle);
    Status(EFICALL *start_image)(Handle image_handle, USize *exit_data_size,
                                 U16 **exit_data);
    Status(EFICALL *exit)(Handle image_handle, Status exit_status,
                          USize exit_data_size, U16 *exit_data);
    Status(EFICALL *unload_image)(Handle image_handle);
    Status(EFICALL *exit_boot_services)(Handle image_handle, USize map_key);

    Status(EFICALL *get_next_monotonic_count)(U64 *count);
    Status(EFICALL *stall)(USize microseconds);
    Status(EFICALL *set_watchdog_timer)(USize timeout, U64 watchdog_code,
                                        USize data_size, U16 *watchdog_data);

    /* 1.1+ */

    Status(EFICALL *connect_controller)(
        Handle controller_handle, Handle *driver_image_handle,
        DevicePathProtocol *remaining_device_path, bool recursive);
    Status(EFICALL *disconnect_controller)(Handle controller_handle,
                                           Handle driver_image_handle,
                                           Handle child_handle);

    Status(EFICALL *open_protocol)(Handle handle, Guid *protocol,
                                   void **interface, Handle agent_handle,
                                   Handle controller_handle, U32 attributes);
    Status(EFICALL *close_protocol)(Handle handle, Guid *protocol,
                                    Handle agent_handle,
                                    Handle controller_handle);
    Status(EFICALL *open_protocol_information)(
        Handle handle, Guid *protocol,
        OpenProtocolInformationEntry **entry_buffer, USize *entry_count);

    Status(EFICALL *protocols_per_handle)(Handle handle,
                                          Guid ***protocol_buffer,
                                          USize *protocol_buffer_count);
    Status(EFICALL *locate_handle_buffer)(LocateSearchType search_type,
                                          Guid *protocol, void *search_key,
                                          USize *no_handles, Handle **buffer);
    Status(EFICALL *locate_protocol)(Guid *protocol, void *registration,
                                     void **interface);
    Status(EFICALL *install_multiple_protocol_interfaces)(Handle *handle, ...);
    Status(EFICALL *uninstall_multiple_protocol_interfaces)(Handle handle, ...);

    Status(EFICALL *calculate_crc32)(void *data, USize data_size, U32 *crc32);

    void(EFICALL *copy_mem)(void *destination, void *source, USize length);
    void(EFICALL *set_mem)(void *buffer, USize size, U8 value);

    /* 2.0+ */

    Status(EFICALL *create_event_ex)(U32 type, Tpl notify_tpl,
                                     EventNotify notify_function,
                                     void *notify_context, Guid *event_group,
                                     Event *event);
} BootServices;

#define SYSTEM_TABLE_SIGNATURE U64_C(0x5453595320494249) /* "IBI SYST" */

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
