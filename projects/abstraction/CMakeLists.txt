cmake_minimum_required(VERSION 3.22)
project(abstraction LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("${REPO_PROJECTS}/setup.cmake")
    include("${REPO_PROJECTS}/print-configuration.cmake")
    include("${REPO_PROJECTS}/abstractions.cmake")
endif()

add_subdirectory("efi/code")

# add_subdirectory(log)
# add_subdirectory(memory)
#
# ## NOTE: temporary until we improve the abstraction structure
# add_subdirectory(physical)
# add_subdirectory(virtual)
# add_subdirectory(interrupts)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    fetch_and_write_project_targets()
endif()
