set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -march=native -m64 -Wall -Wextra -Wconversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-pointer-sign -Wno-sign-conversion -Wdouble-promotion -Wvla -W"
)

if("${BUILD_UNIT_TESTS}")
    add_compile_definitions(BUILD_UNIT_TESTS)
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
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

set(VALID_ENVIRONMENTS "freestanding" "posix")
list(FIND VALID_ENVIRONMENTS ${ENVIRONMENT} VALID_ENVIRONMENT_INDEX)
if(VALID_ENVIRONMENT_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid environment specified. Please choose one of: ${VALID_ENVIRONMENTS}"
    )
endif()
if(${ENVIRONMENT} STREQUAL "freestanding")
    add_compile_definitions(FREESTANDING_ENVIRONMENT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -nostdlib -ffreestanding")
endif()
if(${ENVIRONMENT} STREQUAL "posix")
    add_compile_definitions(POSIX_ENVIRONMENT)
endif()

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
        targets
        DIRECTORY "${currentDir}"
        BUILDSYSTEM_TARGETS
    )
    list(FILTER targets INCLUDE REGEX "^${PROJECT_NAME}.*")
    set(${result} ${${result}} ${targets} PARENT_SCOPE)
endfunction()

function(fetch_and_write_project_targets)
    set(project_targets)
    get_project_targets(project_targets ${CMAKE_CURRENT_BINARY_DIR})

    message(STATUS "00000000000000000000000000000000")
    # Print all collected targets
    foreach(target ${project_targets})
        message(STATUS "Target: ${target}")
    endforeach()

    file(WRITE ${PROJECTS_TARGETS_FILE} "")
    foreach(target ${all_targets})
        file(APPEND ${PROJECTS_TARGETS_FILE} "${target}\n")
    endforeach()
endfunction()
