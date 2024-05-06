project(interoperation LANGUAGES C)

set(MY_LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

add_library(${MY_LIBRARY} ${LIBRARY_TYPE}
    "src/acpi/c-acpi-rsdp.c"
    "src/memory/standard.c"
)

target_include_directories(${MY_LIBRARY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
