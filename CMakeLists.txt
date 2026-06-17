cmake_minimum_required(VERSION 3.22)

project(AllPassPhase VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ALLPASSPHASE_ENABLE_CLANG_TIDY "Run clang-tidy while building AllPassPhase" OFF)

include(FetchContent)

FetchContent_Declare(
	JUCE
	GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
	GIT_TAG 8.0.13
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AllPassPhase
	COMPANY_NAME "enummusic"
	IS_SYNTH FALSE
	NEEDS_MIDI_INPUT FALSE
	NEEDS_MIDI_OUTPUT FALSE
	IS_MIDI_EFFECT FALSE
	EDITOR_WANTS_KEYBOARD_FOCUS FALSE
	COPY_PLUGIN_AFTER_BUILD FALSE
	PLUGIN_MANUFACTURER_CODE EnMu
	PLUGIN_CODE Aph3
	FORMATS VST3
	PRODUCT_NAME "AllPassPhase"
)

target_sources(AllPassPhase
	PRIVATE
		Source/PluginEditor.cpp
		Source/PluginProcessor.cpp
		AllPassFilter.cpp
		HardClip.cpp
		LRCrossoverFilter.cpp
)

target_include_directories(AllPassPhase
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(AllPassPhase
	PUBLIC
		JUCE_WEB_BROWSER=0
		JUCE_USE_CURL=0
		JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AllPassPhase
	PRIVATE
		juce::juce_audio_utils
	PUBLIC
		juce::juce_recommended_config_flags
		juce::juce_recommended_warning_flags
)

if(ALLPASSPHASE_ENABLE_CLANG_TIDY)
	find_program(CLANG_TIDY_EXE NAMES clang-tidy)
	if(CLANG_TIDY_EXE)
		set_target_properties(AllPassPhase PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
	else()
		message(WARNING "ALLPASSPHASE_ENABLE_CLANG_TIDY is ON, but clang-tidy was not found")
	endif()
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
	add_custom_target(format
		COMMAND ${CLANG_FORMAT_EXE} -i
			${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.h
			${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.h
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Formatting AllPassPhase sources"
	)
endif()
