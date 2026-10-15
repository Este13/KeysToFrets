cmake_minimum_required(VERSION 3.22)

project(KeysToFrets VERSION 1.0.0 LANGUAGES C CXX)

set(KEYSTOFRETS_ASIO_SDK_DIR "" CACHE PATH "Path to the Steinberg ASIO SDK (enables ASIO driver support)")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(JUCE_WEBVIEW2_PACKAGE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2" CACHE PATH "" FORCE)
add_subdirectory(third_party/JUCE)

set(KEYSTOFRETS_CORE_SOURCES
    Source/core/AppPaths.cpp
    Source/core/SettingsStore.cpp
    Source/core/ChordGrouper.cpp
    Source/core/DetectionThread.cpp
    Source/core/SessionLogger.cpp
    Source/core/LogReader.cpp)

juce_add_gui_app(KeysToFrets
    PRODUCT_NAME "KeysToFrets"
    COMPANY_NAME "KeysToFrets"
    VERSION ${PROJECT_VERSION}
    NEEDS_WEBVIEW2 TRUE)

target_sources(KeysToFrets PRIVATE
    ${KEYSTOFRETS_CORE_SOURCES}
    Source/Main.cpp
    Source/app/AppController.cpp
    Source/app/AppEvents.cpp
    Source/app/AppHandlers.cpp
    Source/app/MainWindow.cpp
    Source/app/UiBridge.cpp
    Source/app/WebView2Check.cpp
    Source/app/FileDialogs.cpp
    Source/audio/AudioEngine.cpp
    Source/audio/AudioStatus.cpp
    Source/audio/MidiInputManager.cpp
    Source/audio/PluginHost.cpp
    Source/audio/PluginEditorWindow.cpp
    Source/audio/ReplayPlayer.cpp)

target_include_directories(KeysToFrets PRIVATE Source)
target_compile_definitions(KeysToFrets PRIVATE
    JUCE_WEB_BROWSER=1
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_LADSPA=0
    JUCE_PLUGINHOST_LV2=0
    JUCE_USE_CURL=0)

if(KEYSTOFRETS_ASIO_SDK_DIR)
    target_compile_definitions(KeysToFrets PRIVATE JUCE_ASIO=1)
    target_include_directories(KeysToFrets PRIVATE "${KEYSTOFRETS_ASIO_SDK_DIR}/common" "${KEYSTOFRETS_ASIO_SDK_DIR}/host" "${KEYSTOFRETS_ASIO_SDK_DIR}/host/pc")
endif()

if(MSVC)
    target_compile_options(KeysToFrets PRIVATE /utf-8 /MP)
endif()

target_link_libraries(KeysToFrets PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_recommended_warning_flags)

add_custom_target(KeysToFretsAssets
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/ui" "$<TARGET_FILE_DIR:KeysToFrets>/ui"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/third_party/chords-db/LICENSE" "$<TARGET_FILE_DIR:KeysToFrets>/chords-db-LICENSE.txt"
    VERBATIM)
add_dependencies(KeysToFrets KeysToFretsAssets)

juce_add_console_app(KeysToFretsTests PRODUCT_NAME "KeysToFretsTests")
target_sources(KeysToFretsTests PRIVATE
    ${KEYSTOFRETS_CORE_SOURCES}
    tests/cpp/TestMain.cpp
    tests/cpp/ChordGrouperTests.cpp
    tests/cpp/SettingsStoreTests.cpp
    tests/cpp/SessionLoggerTests.cpp
    tests/cpp/DetectionThreadTests.cpp)
target_include_directories(KeysToFretsTests PRIVATE Source)
target_compile_definitions(KeysToFretsTests PRIVATE JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
if(MSVC)
    target_compile_options(KeysToFretsTests PRIVATE /utf-8 /MP)
endif()
target_link_libraries(KeysToFretsTests PRIVATE
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_audio_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
