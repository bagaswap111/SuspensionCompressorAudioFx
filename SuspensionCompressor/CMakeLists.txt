cmake_minimum_required(VERSION 3.15)

project(SuspensionCompressor VERSION 1.0.0)

# Keep plugin installation within the build tree so a normal user account can build it.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Plugin install prefix" FORCE)

# JUCE Framework Setup
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(juce)

# Plugin Source Files
juce_add_plugin(SuspensionCompressor
    COMPANY_NAME "SuspensionAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE SusA
    PLUGIN_CODE SusC
    LV2_URI "https://suspensionaudio.com/suspension-compressor"
    FORMATS VST3 Standalone AU LV2
    PRODUCT_NAME "Suspension Compressor")

# Generate the plugin's JuceHeader.h after the target exists
juce_generate_juce_header(SuspensionCompressor)

target_sources(SuspensionCompressor
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/SuspensionModel.h
        Source/SuspensionPresets.h)

target_compile_definitions(SuspensionCompressor
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SuspensionCompressor
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
