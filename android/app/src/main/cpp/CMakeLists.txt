cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(juce_audio_rn_native LANGUAGES CXX C)

if(WIN32)
    set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE STRING "CMake generator" FORCE)
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "CMake generator platform" FORCE)
endif()

set(JUCE_PATH "${CMAKE_SOURCE_DIR}/JUCE_Projects/JuceAudioEngine/JUCE_Library")

if(EXISTS ${JUCE_PATH})
    message(STATUS "JUCE found at: ${JUCE_PATH}")
    
    if(WIN32)
        set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Don't build JUCE examples")
        set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Don't build JUCE extras") 
        set(JUCE_ENABLE_MODULE_SOURCE_GROUPS OFF CACHE BOOL "Disable module groups")
        set(JUCE_TOOL_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/juce_tools" CACHE PATH "JUCE tools directory")
    else()
        set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Don't build JUCE examples")
        set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Don't build JUCE extras")
        set(JUCE_ENABLE_MODULE_SOURCE_GROUPS OFF CACHE BOOL "Disable module groups")
    endif()
    
    add_subdirectory(${JUCE_PATH} JUCE_build)
    set(JUCE_AVAILABLE TRUE)
else()
    message(WARNING "JUCE not found at: ${JUCE_PATH}")
    set(JUCE_AVAILABLE FALSE)
endif()

add_library(juce_audio_rn_native SHARED
    JuceAudioJNI.cpp
)

if(JUCE_AVAILABLE)
    target_link_libraries(juce_audio_rn_native PRIVATE
        juce::juce_core
        juce::juce_events
    )
    
    target_compile_definitions(juce_audio_rn_native PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="JuceAudioEngine"
        JUCE_APPLICATION_VERSION_STRING="1.0.0"
        JUCE_STANDALONE_APPLICATION=1
        JUCE_HEADLESS_PLUGIN_CLIENT=1
        
        JUCE_USE_ANDROID_OPENSLES=0
        JUCE_USE_ANDROID_OBOE=1
        JUCE_ANDROID=1
        
        JUCE_ALSA_MIDI_ENABLED=0
        JUCE_USE_ANDROID_MIDI_API=0
        JUCE_MIDI_ENABLED=0
        
        JUCE_ANDROID_ACTIVITY_CLASSPATH=""
        JUCE_DISABLE_JUCE_VERSION_PRINTING=1
        
        $<$<PLATFORM_ID:Windows>:JUCE_USE_WIN32_THREADS=1>
        $<$<PLATFORM_ID:Windows>:NOMINMAX=1>
        
        HAS_JUCE=1
    )
else()
    target_compile_definitions(juce_audio_rn_native PRIVATE
        HAS_JUCE=0
    )
endif()

if(WIN32 AND MSVC)
    target_compile_options(juce_audio_rn_native PRIVATE
        /utf-8  # Fix character encoding issues
        /permissive-  # Disable non-conforming code
    )
endif()

if(WIN32 AND MSVC)
    target_compile_options(juce_audio_rn_native PRIVATE
        /utf-8  # Fix character encoding issues
        /permissive-  # Disable non-conforming code
        /EHsc  # Exception handling
        /GR  # RTTI enabled
    )
else()
    target_compile_options(juce_audio_rn_native PRIVATE
        -frtti
        -fexceptions
        -fvisibility=hidden
    )
endif()

set_target_properties(juce_audio_rn_native PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    ANDROID_STL c++_shared
)

target_link_libraries(juce_audio_rn_native PRIVATE
    log
    android
)