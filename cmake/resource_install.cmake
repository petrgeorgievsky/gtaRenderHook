include_guard()

# Installs resources to runtime directory at build time, can be used to copy resources
# (we could introduce cmake install, but I'm not sure it's really needed)
# RESOURCE_DIR - path in which resources are located
# DEST_PATH - path relative to runtime dir, in which we copy resources
function(copy_resources_to_build_dir TARGET_NAME RESOURCE_DIR DEST_PATH)
    # Normalize to forward slashes: CONFIGURE_DEPENDS writes the glob pattern
    # into VerifyGlobs.cmake verbatim, and a backslash is parsed as an escape.
    file(TO_CMAKE_PATH "${RESOURCE_DIR}" RESOURCE_DIR)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" CONFIG_UPPER)
    get_target_property(TARGET_OUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER})
    if (NOT TARGET_OUT_DIR)
        get_target_property(TARGET_OUT_DIR ${TARGET_NAME} BINARY_DIR)
        if (NOT TARGET_OUT_DIR)
            message(FATAL_ERROR "Failed to find output dir of target ${TARGET_NAME}")
            return()
        endif ()
    endif ()
    # CONFIGURE_DEPENDS: adding a new resource (e.g. a shader include) has to
    # re-run CMake, otherwise no copy rule is generated for it and the runtime
    # tree silently keeps the old file set.
    file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${RESOURCE_DIR}/*")
    set(COPIED_RESOURCE_FILES "")
    foreach (RES_PATH ${RESOURCE_FILES})
        file(RELATIVE_PATH REL_PATH "${RESOURCE_DIR}" ${RES_PATH})
        list(APPEND COPIED_RESOURCE_FILES "${TARGET_OUT_DIR}/${DEST_PATH}/${REL_PATH}")
        add_custom_command(
                OUTPUT "${TARGET_OUT_DIR}/${DEST_PATH}/${REL_PATH}"
                COMMAND ${CMAKE_COMMAND} -E copy ${RES_PATH} "${TARGET_OUT_DIR}/${DEST_PATH}/${REL_PATH}"
                DEPENDS ${RES_PATH}
                COMMENT "Copying resource: ${REL_PATH} to ${TARGET_OUT_DIR}/${DEST_PATH}/${REL_PATH}"
        )
    endforeach ()
    target_sources(${TARGET_NAME} PRIVATE ${COPIED_RESOURCE_FILES})
    # Just in case MSBuild tries to compile them, e.g. using shader compiler...
    set_source_files_properties(${COPIED_RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
endfunction()