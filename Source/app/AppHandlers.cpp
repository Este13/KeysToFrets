#include "app/AppController.h"
#include "app/AppEvents.h"
#include "core/AppPaths.h"
#include "core/LogReader.h"

namespace keystofrets
{
    void AppController::dispatch (const juce::String& event, const juce::var& payload)
    {
        if (event == "ready")                onReady();
        else if (event == "selectVst")       selectVst();
        else if (event == "openVstEditor")   openPluginEditor();
        else if (event == "setAudio")        onSetAudio (payload);
        else if (event == "setSetting")      onSetSetting (payload);
        else if (event == "chordResult")     onChordResult (payload);
        else if (event == "replay")          onReplay (payload);
        else if (event == "listLogs")        sendLogList();
        else if (event == "browseLog")       browseLog();
        else if (event == "openLog")         openLogFile (juce::File (payload.getProperty ("path", juce::var()).toString()));
        else if (event == "setMidiDevice")   midi.selectDevice (payload.getProperty ("id", juce::var()).toString());
        else if (event == "setMode")
        {
            const auto mode = payload.getProperty ("mode", juce::var()).toString();
            settingsStore.get().mode = mode == "literal" ? "literal" : "translation";
            settingsStore.save();
        }
    }

    void AppController::onReady()
    {
        uiReady = true;
        emit (events::init, buildInit());
    }

    void AppController::onSetSetting (const juce::var& payload)
    {
        const auto key = payload.getProperty ("key", juce::var()).toString();
        const int value = payload.getProperty ("value", 0);
        auto& settings = settingsStore.get();
        if (key == "groupWindowMs")
        {
            settings.groupWindowMs = juce::jlimit (10, 400, value);
            detection.setWindowMs (settings.groupWindowMs);
        }
        else if (key == "replayLengthMs")
        {
            settings.replayLengthMs = juce::jlimit (50, 5000, value);
        }
        settingsStore.save();
    }

    void AppController::handleHeldNotes (const std::vector<int>& notes)
    {
        emit (events::held, payload::heldNotes (notes));
    }

    void AppController::handleCommittedChord (const CommittedChord& chord)
    {
        const int id = nextChordId++;
        pendingChords[id] = chord;
        while (pendingChords.size() > 256)
            pendingChords.erase (pendingChords.begin());
        const int timestamp = static_cast<int> (chord.timeMs - sessionStartMs + 0.5);
        emit (events::chord, payload::committedChord (id, juce::jmax (0, timestamp), chord));
    }

    void AppController::onChordResult (const juce::var& payload)
    {
        const int id = payload.getProperty ("id", -1);
        const auto found = pendingChords.find (id);
        if (found == pendingChords.end())
            return;

        const auto& chord = found->second;
        const int timestamp = juce::jmax (0, static_cast<int> (chord.timeMs - sessionStartMs + 0.5));
        const auto name = payload.getProperty ("name", juce::var()).toString();
        const auto mode = payload.getProperty ("mode", juce::var()).toString();
        logger.append (timestamp, chord.notes, name, mode);
        pendingChords.erase (found);
    }

    void AppController::onReplay (const juce::var& payload)
    {
        const auto notes = payload::parseNotes (payload.getProperty ("notes", juce::var()));
        if (! notes.empty())
            replay.play (notes, settingsStore.get().replayLengthMs);
    }

    void AppController::sendLogList()
    {
        emit (events::logList, payload::logSummaries (logreader::listSessions (paths::logsDirectory())));
    }

    void AppController::openLogFile (const juce::File& file)
    {
        if (! file.existsAsFile() || ! file.hasFileExtension (".jsonl"))
        {
            showToast ("That log file could not be opened.", true);
            return;
        }
        const auto chords = logreader::readChords (file);
        if (chords.empty())
        {
            showToast ("This log contains no chords.", false);
            return;
        }
        emit (events::logOpened, payload::openedLog (file, chords));
    }

    void AppController::browseLog()
    {
        dialogs.chooseLog (FileDialogs::logStartFolder (settingsStore.get().lastLogFolder),
                           [weak = juce::WeakReference<AppController> (this)] (const juce::File& chosen)
                           {
                               if (weak == nullptr)
                                   return;
                               weak->settingsStore.get().lastLogFolder = chosen.getParentDirectory().getFullPathName();
                               weak->settingsStore.save();
                               weak->openLogFile (chosen);
                           });
    }
}

namespace keystofrets
{
    void AppController::runSelfTestIfRequested()
    {
        if (juce::SystemStats::getEnvironmentVariable ("KEYSTOFRETS_SELFTEST", {}) != "1")
            return;

        const std::vector<std::vector<int>> chords { { 48, 52, 55 }, { 42, 57, 62 }, { 48, 55, 59, 64 }, { 45, 57, 60, 64, 72 } };
        int delay = 3000;
        for (const auto& chord : chords)
        {
            for (const int note : chord)
                juce::Timer::callAfterDelay (delay, [weak = juce::WeakReference<AppController> (this), note]
                {
                    if (weak != nullptr)
                        weak->router.handleIncomingMidiMessage (nullptr, juce::MidiMessage::noteOn (1, note, static_cast<juce::uint8> (96)));
                });
            for (const int note : chord)
                juce::Timer::callAfterDelay (delay + 500, [weak = juce::WeakReference<AppController> (this), note]
                {
                    if (weak != nullptr)
                        weak->router.handleIncomingMidiMessage (nullptr, juce::MidiMessage::noteOff (1, note));
                });
            delay += 1200;
        }
    }
}
