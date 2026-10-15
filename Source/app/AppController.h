#pragma once

#include <map>
#include <memory>
#include <juce_events/juce_events.h>
#include "app/FileDialogs.h"
#include "app/MainWindow.h"
#include "audio/AudioEngine.h"
#include "audio/MidiInputManager.h"
#include "audio/MidiRouter.h"
#include "audio/PluginEditorWindow.h"
#include "audio/PluginHost.h"
#include "audio/ReplayPlayer.h"
#include "core/DetectionThread.h"
#include "core/SessionLogger.h"
#include "core/SettingsStore.h"

namespace keystofrets
{
    class AppController : private juce::Timer
    {
    public:
        AppController();
        ~AppController() override;

        void start();
        void shutdown();

        JUCE_DECLARE_WEAK_REFERENCEABLE (AppController)

    private:
        void dispatch (const juce::String& event, const juce::var& payload);
        void emit (const juce::String& event, const juce::var& payload);
        void timerCallback() override;

        void onReady();
        void onSetAudio (const juce::var& payload);
        void onSetSetting (const juce::var& payload);
        void onChordResult (const juce::var& payload);
        void onReplay (const juce::var& payload);
        void sendLogList();
        void openLogFile (const juce::File& file);
        void browseLog();

        void handleCommittedChord (const CommittedChord& chord);
        void handleHeldNotes (const std::vector<int>& notes);

        void selectVst();
        void vstChosen (const juce::File& chosen);
        void loadVst (const juce::File& file, bool restoreSavedState);
        void openPluginEditor();
        void closePluginEditor();
        void persistPluginState();

        juce::var buildInit();
        juce::var midiStatusVar();
        juce::var vstStatusVar();
        void pushMidiStatus();
        void pushAudioStatus();
        void pushVstStatus();
        void showToast (const juce::String& text, bool isError);
        void rememberMidiDevice();
        void recoverAudioAfterFailure (const juce::String& error);
        void runSelfTestIfRequested();

        SettingsStore settingsStore;
        double sessionStartMs;
        juce::Time sessionStartTime;
        bool uiReady = false;
        juce::String vstNotice;

        DetectionThread::Fifo fifo;
        DetectionThread detection;
        AudioEngine engine;
        MidiRouter router;
        MidiInputManager midi;
        ReplayPlayer replay;
        PluginHost pluginHost;
        FileDialogs dialogs;
        SessionLogger logger;

        std::unique_ptr<juce::AudioPluginInstance> plugin;
        juce::String pluginName;
        juce::File pluginFile;
        std::unique_ptr<PluginEditorWindow> editorWindow;

        UiBridge* bridge = nullptr;
        std::unique_ptr<MainWindow> window;

        std::map<int, CommittedChord> pendingChords;
        int nextChordId = 1;
    };
}
