project(test-framework LANGUAGES C)

set(LIBRARY "${PROJECT_NAME}-${CMAKE_BUILD_TYPE}")
set(LIBRARY_TYPE "STATIC")

add_library(${LIBRARY} ${LIBRARY_TYPE}
    "src/test.c"
    "src/expectations.c"
)

target_include_directories(${LIBRARY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${LIBRARY} PRIVATE util-${CMAKE_BUILD_TYPE})

add_subdirectory(tests)
