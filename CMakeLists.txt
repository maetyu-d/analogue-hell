cmake_minimum_required(VERSION 3.22)
project(AnalogueHellPluginSuite VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

set(AH_SHARED_SOURCES
    src/PluginProcessor.cpp
    src/PluginProcessor.h
    src/PluginEditor.cpp
    src/PluginEditor.h
    src/Shared/AnalogHellDSP.h)

function(add_analogue_hell_plugin target_name plugin_name plugin_code model_id)
    juce_add_plugin(${target_name}
        COMPANY_NAME "Analogue Hell"
        BUNDLE_ID "com.analoguehell.${target_name}"
        PLUGIN_MANUFACTURER_CODE AHLL
        PLUGIN_CODE ${plugin_code}
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "${plugin_name}")

    target_sources(${target_name} PRIVATE ${AH_SHARED_SOURCES})

    target_compile_definitions(${target_name} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        AH_PLUGIN_NAME=\"${plugin_name}\"
        AH_MODEL_ID=${model_id})

    target_link_libraries(${target_name} PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
endfunction()

add_analogue_hell_plugin(AHTapeSystem        "AH Tape System"        ATS1 1)
add_analogue_hell_plugin(AHBbdGrime          "AH BBD Grime"          ABD2 2)
add_analogue_hell_plugin(AHCoreBend          "AH Core Bend"          ACB3 3)
add_analogue_hell_plugin(AHTubeMisuse        "AH Tube Misuse"        ATM4 4)
add_analogue_hell_plugin(AHFaultFuzz         "AH Fault Fuzz"         AFF5 5)
add_analogue_hell_plugin(AHBusOverload       "AH Bus Overload"       ABO6 6)
add_analogue_hell_plugin(AHUnstableFilter    "AH Unstable Filter"    AUF7 7)
add_analogue_hell_plugin(AHSpringChaos       "AH Spring Chaos"       ASC8 8)
add_analogue_hell_plugin(AHVinylWeapon       "AH Vinyl Weapon"       AVW9 9)
add_analogue_hell_plugin(AHCalLabGhost       "AH CalLab Ghost"       ACL0 10)

add_executable(loudness_calibrate
    tools/loudness_calibrate.cpp
    src/Shared/AnalogHellDSP.h)

target_link_libraries(loudness_calibrate PRIVATE
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
