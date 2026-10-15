cmake_minimum_required(VERSION 3.22)
project(AxellDuoCompressor VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.15
  GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AxellDuoCompressor
  COMPANY_NAME              "Axell Yobag"
  COMPANY_EMAIL             "axellpetre@protonmail.com"
  BUNDLE_ID                 com.axellyobag.axellduocompressor
  PLUGIN_MANUFACTURER_CODE  Dcmp
  PLUGIN_CODE               Dvc1
  FORMATS                   VST3
  PRODUCT_NAME              "Axell Duo Compressor"
  IS_SYNTH                  FALSE
  NEEDS_MIDI_INPUT          FALSE
  NEEDS_MIDI_OUTPUT         FALSE
  IS_MIDI_EFFECT            FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD   FALSE
  VST3_CATEGORIES           Fx Dynamics)

juce_add_binary_data(DuoBinaryData SOURCES
  Resources/fonts/Inter-Regular.otf
  Resources/fonts/Inter-SemiBold.otf
  Resources/fonts/Inter-Bold.otf)
set_target_properties(DuoBinaryData PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(AxellDuoCompressor PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)

target_compile_definitions(AxellDuoCompressor PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AxellDuoCompressor
  PRIVATE
    DuoBinaryData
    juce::juce_audio_utils
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

#-------------------------------------------------------------------------------
# DSP test suite (JUCE-free, tests Source/Engine.h)
option(DUO_BUILD_TESTS "Build the DuoDspTests suite" ON)
option(DUO_SANITIZE "Build DuoDspTests with AddressSanitizer + UBSan" OFF)
if(DUO_BUILD_TESTS)
  enable_testing()
  find_package(Threads REQUIRED)
  add_executable(DuoDspTests tests/DspTests.cpp)
  target_compile_features(DuoDspTests PRIVATE cxx_std_17)
  target_compile_options(DuoDspTests PRIVATE -Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion
                         $<$<CONFIG:Release>:-O3>)
  target_compile_definitions(DuoDspTests PRIVATE
                             DUO_TEST_BUILD_INFO="${CMAKE_BUILD_TYPE}$<$<BOOL:${DUO_SANITIZE}>:+ASan+UBSan>")
  target_link_libraries(DuoDspTests PRIVATE Threads::Threads)
  if(DUO_SANITIZE)
    target_compile_options(DuoDspTests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer
                           -fno-sanitize-recover=undefined)
    target_link_options(DuoDspTests PRIVATE -fsanitize=address,undefined)
  endif()
  add_test(NAME dsp_suite
           COMMAND DuoDspTests --report ${CMAKE_SOURCE_DIR}/tests/report.md
           WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  set_tests_properties(dsp_suite PROPERTIES TIMEOUT 3600)
endif()

#-------------------------------------------------------------------------------
# Headless GUI snapshot tool (renders the real editor to PNG)
option(DUO_BUILD_SNAPSHOT "Build the DuoGuiSnapshot tool" ON)
if(DUO_BUILD_SNAPSHOT)
  juce_add_console_app(DuoGuiSnapshot PRODUCT_NAME "DuoGuiSnapshot")
  target_sources(DuoGuiSnapshot PRIVATE tests/GuiSnapshot.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_compile_definitions(DuoGuiSnapshot PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1 JucePlugin_Name="Axell Duo Compressor")
  target_link_libraries(DuoGuiSnapshot PRIVATE DuoBinaryData juce::juce_audio_utils juce::juce_dsp
    juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()

#-------------------------------------------------------------------------------
# Plugin-level tests (JUCE): parameters, state, latency, presets, A/B, editor
if(DUO_BUILD_TESTS)
  juce_add_console_app(DuoPluginTests PRODUCT_NAME "DuoPluginTests")
  target_sources(DuoPluginTests PRIVATE tests/PluginTests.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_compile_definitions(DuoPluginTests PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1 JucePlugin_Name="Axell Duo Compressor")
  target_link_libraries(DuoPluginTests PRIVATE DuoBinaryData juce::juce_audio_utils juce::juce_dsp
    juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
  add_test(NAME plugin_suite COMMAND DuoPluginTests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  if(DUO_SANITIZE)
    target_compile_options(DuoPluginTests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer
                           -fno-sanitize-recover=undefined)
    target_link_options(DuoPluginTests PRIVATE -fsanitize=address,undefined)
  endif()
endif()

#-------------------------------------------------------------------------------
# Independent black-box audit host (tests/independent): loads the built .vst3 through JUCE's VST3 hosting
# code, never the plugin's classes (same host as Axell Penta EQ's audit)
option(DUO_BUILD_AUDIT_HOST "Build the DuoAuditHost tool" ON)
if(DUO_BUILD_AUDIT_HOST)
  juce_add_console_app(DuoAuditHost PRODUCT_NAME "DuoAuditHost")
  target_sources(DuoAuditHost PRIVATE tests/independent/AuditHost.cpp)
  target_compile_definitions(DuoAuditHost PRIVATE JUCE_PLUGINHOST_VST3=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_MODAL_LOOPS_PERMITTED=1)
  target_link_libraries(DuoAuditHost PRIVATE juce::juce_audio_utils juce::juce_audio_processors
    juce::juce_recommended_config_flags)
endif()
