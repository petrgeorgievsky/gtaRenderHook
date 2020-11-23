
function(install_rw_library)
    target_link_libraries(${PROJECT_NAME} rw_rh_engine_lib)

    # Copy shaders to build destination
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/rw_rh_engine_lib/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

endfunction()