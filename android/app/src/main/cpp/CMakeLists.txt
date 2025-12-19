cmake_minimum_required(VERSION 3.22.1)
project(yokoi_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Repo root from android/app/src/main/cpp
set(YOKOI_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")

file(GLOB_RECURSE YOKOI_SM5XX_SRC
    "${YOKOI_ROOT}/source/SM5XX/*.cpp"
)

file(GLOB_RECURSE YOKOI_STD_SRC
    "${YOKOI_ROOT}/source/std/*.cpp"
)

file(GLOB_RECURSE YOKOI_GW_ROM_SRC
    "${YOKOI_ROOT}/source/std/GW_ROM/*.cpp"
)

# Exclude 3DS-only/example sources if they slip in
list(FILTER YOKOI_STD_SRC EXCLUDE REGEX ".*\\/sound_example\\/.*")

add_library(yokoi SHARED
    yokoi_jni.cpp
    ${YOKOI_SM5XX_SRC}
    ${YOKOI_STD_SRC}
    ${YOKOI_GW_ROM_SRC}
)

target_include_directories(yokoi PRIVATE
    "${YOKOI_ROOT}/source"
    "${YOKOI_ROOT}/source/std"
    "${YOKOI_ROOT}/source/std/GW_ROM"
    "${YOKOI_ROOT}/source/SM5XX"
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)

target_link_libraries(yokoi
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${glesv3-lib}
)
