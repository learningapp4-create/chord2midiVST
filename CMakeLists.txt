cmake_minimum_required(VERSION 3.22)

project(Chord2MIDI VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(chord2midi_core STATIC
    src/Styles.cpp
    src/Phrase.cpp
    src/SmfWriter.cpp
)
target_include_directories(chord2midi_core PUBLIC src)
target_compile_options(chord2midi_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(phrase_tests tests/phrase_tests.cpp)
target_link_libraries(phrase_tests PRIVATE chord2midi_core)
target_compile_options(phrase_tests PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME phrase COMMAND phrase_tests)

option(CHORD2MIDI_BUILD_PLUGIN "Configure the JUCE VST3 and standalone targets" ON)

if(CHORD2MIDI_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)

    juce_add_plugin(Chord2MIDI
        COMPANY_NAME "Chord2MIDI"
        BUNDLE_ID com.chord2midi.phrase
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT TRUE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE C2MD
        PLUGIN_CODE CH2M
        FORMATS VST3 Standalone
        PRODUCT_NAME "Chord2MIDI"
    )

    juce_generate_juce_header(Chord2MIDI)

    target_sources(Chord2MIDI PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
    )

    target_compile_definitions(Chord2MIDI PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(Chord2MIDI PRIVATE
        chord2midi_core
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
    )
endif()
