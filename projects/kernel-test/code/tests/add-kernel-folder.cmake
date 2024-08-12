function(addKernelFolder kernelFolder)
    set(KERNEL_SOURCE_LOCATION "${REPO_PROJECTS}/kernel/code/${kernelFolder}")
    set(KERNEL_BINARY_LOCATION "${CMAKE_BINARY_DIR}/${kernelFolder}")

    add_subdirectory("${KERNEL_SOURCE_LOCATION}" "${KERNEL_BINARY_LOCATION}")
    add_dependencies(${TESTS} "${kernelFolder}-${CMAKE_BUILD_TYPE}")
    target_include_directories(
        ${TESTS}
        PUBLIC "${KERNEL_SOURCE_LOCATION}/include"
    )
    target_link_libraries(
        ${TESTS}
        PRIVATE
            "${KERNEL_BINARY_LOCATION}/lib${kernelFolder}-${CMAKE_BUILD_TYPE}.a"
    )
endfunction()
