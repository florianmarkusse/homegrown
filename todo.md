- Fix code so that it compiles :)
- remove **asm** in favor of asm and **volatile** for volatile
- Rething platform-abstraction fault -> should just have single functions for each "interrupt":
  - noPhysicalMemory();
  - etc.
  - ask iwyu crew how to handle platform-abstraction includes? Perhap
- Figure out what to do with memory allocation & mapping in os-loader
- Should we even have a platfor-abstraction/cpu.h ??? It seems too low-level
- Fix posix tests in physical memory
- Create macro for message and exit in efi just like flush after but with wait key stuff
- can I get rid of EFICALL ? I am compiling with efi stuff anyway which automatically does the right ABI afaik
- Rethink memory allocation for kernel structures in uefi and whether or not to add them to free physical memory in kernel --- definitely some bugs now.
- if you ever decide to use unit tests in the kernel, need to create an abstraction for test-framework so it can be used in both posix and freestanding environments

CPU features to implement/turn on in x86

- fpu
- sse
- avx
- gpe

project(platform-abstraction-log LANGUAGES C ASM)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
${PROJECT_NAME}
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if("${ENVIRONMENT}" STREQUAL "freestanding")
    include_and_link_object_library(kernel-log)
elseif("${ENVIRONMENT}" STREQUAL "efi")
include_and_link_object_library(efi-log)
elseif("${ENVIRONMENT}" STREQUAL "posix")
include_and_link_object_library(posix-log)
else()
message(FATAL_ERROR "Could not match ENVIRONMENT variable")
endif()

target_include_directories(
${AGGREGATED_INTERFACE}
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
