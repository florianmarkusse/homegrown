set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -march=native -m64 -Wall -Wextra -Wconversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-pointer-sign -Wno-sign-conversion -Wdouble-promotion -Wvla"
)

if("${UNIT_TEST_BUILD}")
    add_compile_definitions(UNIT_TEST_BUILD)
else()
    add_compile_definitions(PROJECT_BUILD)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING
        "Build type (Debug, Release, Profiling, Fuzzing)"
        FORCE
    )
endif()
set(VALID_BUILD_TYPES "Debug" "Release" "Profiling" "Fuzzing")
list(FIND VALID_BUILD_TYPES ${CMAKE_BUILD_TYPE} VALID_BUILD_TYPE_INDEX)
if(VALID_BUILD_TYPE_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid build type specified. Please choose one of: ${VALID_BUILD_TYPES}"
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Fuzzing" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
    add_compile_definitions("DEBUG")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Profiling")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -O2 -pg")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # TODO: Add -flto on production build I guess or on flag?
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto")
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

set(VALID_ENVIRONMENTS "freestanding" "posix" "efi")
list(FIND VALID_ENVIRONMENTS ${ENVIRONMENT} VALID_ENVIRONMENT_INDEX)
if(VALID_ENVIRONMENT_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid environment specified. Please choose one of: ${VALID_ENVIRONMENTS}"
    )
endif()
if(${ENVIRONMENT} STREQUAL "freestanding")
    add_compile_definitions(FREE_C_LIB)
    add_compile_definitions(FREESTANDING_ENVIRONMENT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -nostdlib -ffreestanding")
    add_link_options("--ld-path=${CMAKE_LINKER}")
endif()
if(${ENVIRONMENT} STREQUAL "efi")
    add_compile_definitions(FREE_C_LIB)
    add_compile_definitions(EFI_ENVIRONMENT)
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -nostdinc -target x86_64-unknown-windows -mgeneral-regs-only -mno-stack-arg-probe"
    )
    get_filename_component(LINKER_FILENAME ${CMAKE_LINKER} NAME)
    add_link_options(
        -fuse-ld=${LINKER_FILENAME}
        -Wl,-entry:efi_main,-subsystem:efi_application
    )
endif()
if(${ENVIRONMENT} STREQUAL "posix")
    add_compile_definitions(HOSTED_C_LIB)
    add_compile_definitions(POSIX_ENVIRONMENT)
    add_link_options("--ld-path=${CMAKE_LINKER}")
endif()

set(VALID_ARCHITECTURES "x86" "mock")
list(FIND VALID_ARCHITECTURES ${ARCHITECTURE} VALID_ARCHITECTURE_INDEX)
if(VALID_ARCHITECTURE_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid architecture specified. Please choose one of: ${VALID_ARCHITECTURES}"
    )
endif()
if(${ARCHITECTURE} STREQUAL "x86")
    add_compile_definitions(X86_ARCHITECTURE)
endif()
if(${ARCHITECTURE} STREQUAL "posix")
    add_compile_definitions(MOCK_ARHCITECTURE)
endif()

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

function(add_subproject project)
    add_subdirectory(
        "${REPO_PROJECTS}/${project}/code"
        "${REPO_PROJECTS}/${project}/code/${BUILD_OUTPUT_PATH}"
    )
endfunction()

function(get_project_targets result currentDir)
    get_property(
        subdirectories
        DIRECTORY "${currentDir}"
        PROPERTY SUBDIRECTORIES
    )
    foreach(subdirectory IN LISTS subdirectories)
        get_project_targets(${result} "${subdirectory}")
    endforeach()
    get_directory_property(
        all_targets
        DIRECTORY "${currentDir}"
        BUILDSYSTEM_TARGETS
    )
    set(buildable_targets)
    foreach(target IN LISTS all_targets)
        get_property(target_type TARGET ${target} PROPERTY TYPE)
        if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
            list(APPEND buildable_targets ${target})
        endif()
    endforeach()
    list(FILTER buildable_targets INCLUDE REGEX "^${PROJECT_NAME}.*")
    set(${result} ${${result}} ${buildable_targets} PARENT_SCOPE)
endfunction()

function(fetch_and_write_project_targets)
    set(project_targets)
    get_project_targets(project_targets ${CMAKE_CURRENT_BINARY_DIR})

    file(WRITE ${PROJECT_TARGETS_FILE} "")
    foreach(target ${project_targets})
        file(APPEND ${PROJECT_TARGETS_FILE} "${target}\n")
    endforeach()
endfunction()

function(add_correct_platfom_abstraction_implementations)
    if("${ENVIRONMENT}" STREQUAL "freestanding")
        if("${ARCHITECTURE}" STREQUAL "x86")
            add_subproject("x86")
        endif()
        if("${ARCHITECTURE}" STREQUAL "mock")
            add_subproject("mock")
        endif()
        add_subproject("free-c")
    elseif("${ENVIRONMENT}" STREQUAL "efi")
        if("${ARCHITECTURE}" STREQUAL "x86")
            add_subproject("x86")
        endif()
        if("${ARCHITECTURE}" STREQUAL "mock")
            add_subproject("mock")
        endif()
        add_subproject("free-c")
    elseif("${ENVIRONMENT}" STREQUAL "posix")
        if("${ARCHITECTURE}" STREQUAL "mock")
            add_subproject("mock")
        endif()
        add_subproject("posix")
    else()
        message(FATAL_ERROR "Could not match ENVIRONMENT variable")
    endif()
endfunction()

function(add_platform_abstraction_and_correct_implementations)
    add_correct_platfom_abstraction_implementations()
    add_subproject("platform-abstraction")
endfunction()
