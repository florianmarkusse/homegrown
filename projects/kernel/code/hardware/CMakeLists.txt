project(hardware LANGUAGES C ASM)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

add_library(${LIBRARY} ${LIBRARY_TYPE}
    "src/idt.c"
    "src/idts.S"
)

target_include_directories(${LIBRARY} PUBLIC "${REPO_ROOT}/projects/interoperation/code/include")

target_include_directories(${LIBRARY}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIBRARY} PUBLIC util-${CMAKE_BUILD_TYPE})
