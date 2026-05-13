# Brings the SFSE source tree into the build.
#
# Resolution order:
#   1. -DSFSE_LOCAL_PATH=<path>  (CMake cache variable)
#   2. SFSE_LOCAL_PATH environment variable
#   3. FetchContent from https://github.com/ianpatt/sfse at SFSE_GIT_TAG
#
# In all cases we end up with a usable sfse::sfse_common target.

include_guard(GLOBAL)

set(SFSE_GIT_REPOSITORY "https://github.com/ianpatt/sfse.git" CACHE STRING "SFSE git repository URL")
set(SFSE_GIT_TAG        "v0.2.19"                             CACHE STRING "SFSE git tag/branch/commit")

if(NOT SFSE_LOCAL_PATH AND DEFINED ENV{SFSE_LOCAL_PATH})
    set(SFSE_LOCAL_PATH "$ENV{SFSE_LOCAL_PATH}" CACHE PATH "Path to a local SFSE source checkout")
endif()

set(_sfse_root "")

if(SFSE_LOCAL_PATH)
    if(NOT EXISTS "${SFSE_LOCAL_PATH}/sfse_common/CMakeLists.txt")
        message(FATAL_ERROR
            "SFSE_LOCAL_PATH=\"${SFSE_LOCAL_PATH}\" does not look like an SFSE source tree "
            "(expected sfse_common/CMakeLists.txt). Point it at the directory that contains "
            "the sfse/, sfse_common/, and xbyak/ folders.")
    endif()
    set(_sfse_root "${SFSE_LOCAL_PATH}")
    message(STATUS "sfse-template-plugin: using local SFSE at ${_sfse_root}")
else()
    include(FetchContent)
    # SOURCE_SUBDIR restricts FetchContent_MakeAvailable to the sfse_common
    # CMakeLists, so we don't pull in the sfse SHARED target or sfse_loader.exe.
    FetchContent_Declare(
        sfse
        GIT_REPOSITORY ${SFSE_GIT_REPOSITORY}
        GIT_TAG        ${SFSE_GIT_TAG}
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  sfse_common
    )
    message(STATUS "sfse-template-plugin: fetching SFSE ${SFSE_GIT_TAG} from ${SFSE_GIT_REPOSITORY}")
    FetchContent_MakeAvailable(sfse)
    set(_sfse_root "${sfse_SOURCE_DIR}")
    set(_sfse_already_added TRUE)
endif()

# Local-path case: we still need to add sfse_common ourselves. The FetchContent
# branch already did this via SOURCE_SUBDIR.
#
# Why only sfse_common: its public include dirs cover both sfse/ and sfse_common/
# headers (e.g. <sfse/PluginAPI.h>), which is all a plugin needs. Pulling in the
# sfse SHARED target would build the SFSE runtime DLL itself and force a link-time
# dependency on its import library, which plugins do not want.
if(NOT _sfse_already_added AND NOT TARGET sfse_common)
    add_subdirectory("${_sfse_root}/sfse_common" "${CMAKE_BINARY_DIR}/_sfse_common_build" EXCLUDE_FROM_ALL)
endif()
