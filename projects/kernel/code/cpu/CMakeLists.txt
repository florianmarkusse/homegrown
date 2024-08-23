project(cpu LANGUAGES C ASM)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

set(SOURCE_FILES)

if("${UNIT_TEST_BUILD}")
    list(APPEND SOURCE_FILES "src/posix/idt.c")
else()
    list(APPEND SOURCE_FILES "src/kernel/idt.c" "src/kernel/idts.S")
endif()

add_library(${LIBRARY} ${LIBRARY_TYPE} ${SOURCE_FILES})

target_include_directories(
    ${LIBRARY}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_include_directories(${LIBRARY} PUBLIC "${CMAKE_SOURCE_DIR}/util/include")

target_include_directories(
    ${LIBRARY}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIBRARY} PUBLIC memory-manipulation-${CMAKE_BUILD_TYPE})
target_link_libraries(${LIBRARY} PUBLIC log-${CMAKE_BUILD_TYPE})
target_link_libraries(${LIBRARY} PUBLIC text-${CMAKE_BUILD_TYPE})
