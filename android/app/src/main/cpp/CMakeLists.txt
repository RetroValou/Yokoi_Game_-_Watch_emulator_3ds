cmake_minimum_required(VERSION 3.22.1)
project(yokoi_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(YOKOI_EMBEDDED_ASSETS "Build with compiled-in ROMs/assets (embedded build)" OFF)

# Minimum external pack content version that this build accepts.
# Bump this when the app expects newer pack contents (textures/layout/etc.).
set(YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION "3" CACHE STRING "Minimum required ROM pack content version")

# Repo root from android/app/src/main/cpp
set(YOKOI_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")

file(GLOB_RECURSE YOKOI_SM5XX_SRC CONFIGURE_DEPENDS
    "${YOKOI_ROOT}/source/SM5XX/*.cpp"
)

file(GLOB_RECURSE YOKOI_STD_SRC CONFIGURE_DEPENDS
    "${YOKOI_ROOT}/source/std/*.cpp"
)

# NOTE: The broad std glob will also pick up generated ROM sources under
# source/std/GW_ROM and source/std/GW_ROM_RGDS. We compile only one set
# explicitly below, so exclude both from the std list to avoid duplicate symbols.
list(FILTER YOKOI_STD_SRC EXCLUDE REGEX ".*[/\\\\]GW_ROM[/\\\\].*")
list(FILTER YOKOI_STD_SRC EXCLUDE REGEX ".*[/\\\\]GW_ROM_RGDS[/\\\\].*")

file(GLOB_RECURSE YOKOI_GW_ROM_SRC CONFIGURE_DEPENDS
    "${YOKOI_ROOT}/source/std/GW_ROM_RGDS/*.cpp"
)

if(NOT YOKOI_EMBEDDED_ASSETS)
    set(YOKOI_GW_ROM_SRC "")
endif()

# Exclude 3DS-only/example sources if they slip in
list(FILTER YOKOI_STD_SRC EXCLUDE REGEX ".*\\/sound_example\\/.*")

add_library(yokoi SHARED
    core/yokoi_app_flow.cpp
    core/yokoi_audio.cpp
    core/yokoi_cpu_utils.cpp
    core/yokoi_emulation_thread.cpp
    core/yokoi_game_loader.cpp
    core/yokoi_layout.cpp
    core/yokoi_menu_selection.cpp
    core/yokoi_runtime_state.cpp
    core/yokoi_segments_state.cpp

    input/yokoi_controller_state.cpp
    input/yokoi_input_mapping.cpp

    render/yokoi_gl.cpp

    jni/yokoi_controller_jni.cpp
    jni/yokoi_jni.cpp
    jni/yokoi_jni_aliases.cpp
    jni/yokoi_settings_jni.cpp
    ${YOKOI_SM5XX_SRC}
    ${YOKOI_STD_SRC}
    ${YOKOI_GW_ROM_SRC}
)

if(YOKOI_EMBEDDED_ASSETS)
    target_compile_definitions(yokoi PRIVATE YOKOI_EMBEDDED_ASSETS=1)
endif()

target_compile_definitions(yokoi PRIVATE YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION=${YOKOI_ROMPACK_REQUIRED_CONTENT_VERSION})

target_include_directories(yokoi PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/core"
    "${CMAKE_CURRENT_LIST_DIR}/input"
    "${CMAKE_CURRENT_LIST_DIR}/render"
    "${CMAKE_CURRENT_LIST_DIR}/jni"
    "${YOKOI_ROOT}/source"
    "${YOKOI_ROOT}/source/std"
    "${YOKOI_ROOT}/source/std/GW_ROM_RGDS"
    "${YOKOI_ROOT}/source/SM5XX"
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)
find_library(aaudio-lib aaudio)

target_link_libraries(yokoi
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${glesv3-lib}
    ${aaudio-lib}
)
