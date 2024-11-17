cmake_minimum_required(VERSION 3.22)
project(shared LANGUAGES C ASM)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include("../../setup.cmake")
    include("../../print-configuration.cmake")
    add_subdirectory(
        "${REPO_PROJECTS}/platform-abstraction/code"
        "${CMAKE_CURRENT_BINARY_DIR}/platform-abstraction"
    )
endif()

add_subdirectory(text)
add_subdirectory(memory)
add_subdirectory(log)
add_subdirectory(maths)
add_subdirectory(buffer)
add_subdirectory(hash)
add_subdirectory(dynamic-array)

# install(
#     EXPORT sharedTargets
#     FILE sharedTargets.cmake
#     NAMESPACE "${PROJECT_NAME}::"
#     DESTINATION lib/cmake/shared
# )
