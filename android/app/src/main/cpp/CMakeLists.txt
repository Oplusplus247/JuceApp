cmake_minimum_required(VERSION 3.22.1)
project(juce_audio_rn_native LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Paths: adjust only if your tree is different ----
set(JUCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/JUCE)
set(OBOE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/oboe)

if(NOT EXISTS "${JUCE_DIR}/modules/juce_core/juce_core.cpp")
    message(FATAL_ERROR "JUCE not found at ${JUCE_DIR} (expected JUCE/modules/*)")
endif()
if(NOT EXISTS "${OBOE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Oboe not found at ${OBOE_DIR} (expected oboe/CMakeLists.txt)")
endif()

# ---- Oboe (I/O) ----
add_subdirectory(${OBOE_DIR} oboe_build)

# ---- Native library ----
add_library(juce_audio_rn_native SHARED
    JuceAudioJNI.cpp
)

# JUCE modules (core/basics/formats/dsp)
target_sources(juce_audio_rn_native PRIVATE
    ${JUCE_DIR}/modules/juce_core/juce_core.cpp
    ${JUCE_DIR}/modules/juce_audio_basics/juce_audio_basics.cpp
    ${JUCE_DIR}/modules/juce_audio_formats/juce_audio_formats.cpp
    ${JUCE_DIR}/modules/juce_dsp/juce_dsp.cpp
)

# Headers
target_include_directories(juce_audio_rn_native
    PRIVATE
        ${JUCE_DIR}/modules
)

# NDK cpu-features (header path + source or lib)
if(DEFINED ANDROID_NDK)
    # 1) Header include path so <cpu-features.h> resolves
    if(EXISTS "${ANDROID_NDK}/sources/android/cpufeatures")
        target_include_directories(juce_audio_rn_native PRIVATE
            ${ANDROID_NDK}/sources/android/cpufeatures
        )
    endif()

    # 2) Some NDKs don’t ship a separate lib; compile the source directly if present
    if(EXISTS "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
        target_sources(juce_audio_rn_native PRIVATE
            ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c
        )
    endif()
endif()

# Compile defs: headless + module markers
target_compile_definitions(juce_audio_rn_native PRIVATE
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JUCE_ANDROID=1
    JUCE_STANDALONE_APPLICATION=0
    JUCE_DSP_MODULE_AVAILABLE=1
    JUCE_MODULE_AVAILABLE_juce_core=1
    JUCE_MODULE_AVAILABLE_juce_audio_basics=1
    JUCE_MODULE_AVAILABLE_juce_audio_formats=1
    JUCE_MODULE_AVAILABLE_juce_dsp=1
)

# System libs
find_library(log-lib log)
find_library(android-lib android)
find_library(opensles-lib OpenSLES)
find_library(cpufeatures-lib cpufeatures) # if NDK still exposes this lib

set(_libs ${log-lib} ${android-lib} ${opensles-lib})
if(cpufeatures-lib)
    list(APPEND _libs ${cpufeatures-lib})
endif()

# ---- Link ----
target_link_libraries(juce_audio_rn_native PRIVATE
    oboe
    ${_libs}
)
