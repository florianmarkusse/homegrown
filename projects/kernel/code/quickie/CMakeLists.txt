cmake_minimum_required(VERSION 3.28)
project(quickie LANGUAGES C ASM)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

message(WARNING "I AM DYING heree")

if(UNIT_TEST)
    message(WARNING "Unit testing is enabled")
else()
    message(WARNING "Unit testing is disabled")
endif()

add_library(${LIBRARY} ${LIBRARY_TYPE} "src/hello.c")

target_include_directories(
    ${LIBRARY}
    PUBLIC "${REPO_ROOT}/projects/interoperation/code/include"
)

target_include_directories(
    ${LIBRARY}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
