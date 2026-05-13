# Optional post-build deploy: if STARFIELD_PATH is set (as a CMake var or env var)
# the freshly built plugin DLL is copied into <STARFIELD_PATH>/Data/SFSE/Plugins/
# after every successful build.
#
# Usage:
#   include(DeployPlugin)
#   sfse_template_deploy_plugin(<target>)

include_guard(GLOBAL)

function(sfse_template_deploy_plugin target)
    if(NOT STARFIELD_PATH AND DEFINED ENV{STARFIELD_PATH})
        set(STARFIELD_PATH "$ENV{STARFIELD_PATH}")
    endif()

    if(NOT STARFIELD_PATH)
        message(STATUS "sfse-template-plugin: STARFIELD_PATH not set - skipping post-build deploy")
        return()
    endif()

    set(_dest "${STARFIELD_PATH}/Data/SFSE/Plugins")
    message(STATUS "sfse-template-plugin: post-build deploy enabled -> ${_dest}")

    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_dest}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${target}>" "${_dest}/"
        COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_PDB_FILE:${target}>>,copy_if_different,true>
                "$<TARGET_PDB_FILE:${target}>" "${_dest}/"
        COMMENT "Deploying ${target} to ${_dest}"
        VERBATIM
    )
endfunction()
