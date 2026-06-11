include_guard()
include(resource_install)

function(install_rw_library)
    target_link_libraries(${PROJECT_NAME} rw_rh_engine_lib)

    copy_resources_to_build_dir(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/rw_rh_engine_lib/resources" "resources")

endfunction()