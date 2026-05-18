cmake_minimum_required(VERSION 3.22)
project(GrooveDigger VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find JUCE ────────────────────────────────────────────────────────────────
# Place the JUCE repo (or a symlink) at GrooveDigger/JUCE/, OR install JUCE
# system-wide so find_package can locate it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(GrooveDigger
    VERSION                     1.0.0
    COMPANY_NAME                "GrooveDigger"
    BUNDLE_ID                   "com.groovedigger.plugin"
    PLUGIN_MANUFACTURER_CODE    Grdg
    PLUGIN_CODE                 Grdg
    FORMATS                     AU VST3 Standalone
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_NAME                 "Groove Digger"
    DESCRIPTION                 "Bassline generator with internal mono synth"
    VST3_CATEGORIES             "Instrument|Generator"
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
)

juce_generate_juce_header(GrooveDigger)

target_sources(GrooveDigger PRIVATE
    Source/Parameters.cpp
    Source/MusicTheory.cpp
    Source/GrooveEngine.cpp
    Source/MonoSynth.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(GrooveDigger PRIVATE Source)

target_compile_definitions(GrooveDigger PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(GrooveDigger
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
