#include <juce_core/juce_core.h>
#include "core/SettingsStore.h"

struct SettingsStoreTests : public juce::UnitTest
{
    SettingsStoreTests() : juce::UnitTest ("SettingsStore", "KeysToFrets") {}

    void runTest() override
    {
        using namespace keystofrets;
        auto directory = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("keystofrets_settings_test");
        directory.deleteRecursively();
        const auto file = directory.getChildFile ("settings.json");

        beginTest ("missing file gives defaults and starts with an empty VST");
        {
            SettingsStore store (file);
            store.load();
            expect (store.get().lastVstPath.isEmpty());
            expect (store.get().lastVstFolder.isEmpty());
            expectEquals (store.get().groupWindowMs, 60);
            expectEquals (store.get().mode, juce::String ("translation"));
            expect (! store.get().window.valid);
        }

        beginTest ("every field survives a save and load");
        {
            SettingsStore store (file);
            auto& s = store.get();
            s.lastVstPath = "C:\\Plugins\\Synth.vst3";
            s.lastVstFolder = "C:\\Plugins";
            s.lastLogFolder = "D:\\logs";
            s.midiDeviceId = "id-1";
            s.midiDeviceName = "Keyboard";
            s.vstState = "QUJD";
            s.mode = "literal";
            s.groupWindowMs = 80;
            s.replayLengthMs = 900;
            s.audio = { "Windows Audio (Exclusive Mode)", "Speakers", 48000.0, 128 };
            s.window = { true, true, 10, 20, 1000, 700 };
            expect (store.save());

            SettingsStore loaded (file);
            loaded.load();
            const auto& r = loaded.get();
            expectEquals (r.lastVstPath, s.lastVstPath);
            expectEquals (r.lastVstFolder, s.lastVstFolder);
            expectEquals (r.lastLogFolder, s.lastLogFolder);
            expectEquals (r.midiDeviceId, s.midiDeviceId);
            expectEquals (r.midiDeviceName, s.midiDeviceName);
            expectEquals (r.vstState, s.vstState);
            expectEquals (r.mode, s.mode);
            expectEquals (r.groupWindowMs, 80);
            expectEquals (r.replayLengthMs, 900);
            expectEquals (r.audio.type, s.audio.type);
            expectEquals (r.audio.device, s.audio.device);
            expectEquals (r.audio.sampleRate, 48000.0);
            expectEquals (r.audio.bufferSize, 128);
            expect (r.window.valid);
            expect (r.window.maximised);
            expectEquals (r.window.width, 1000);
        }

        beginTest ("corrupt or out of range values fall back safely");
        {
            file.replaceWithText ("{\"groupWindowMs\": 99999, \"mode\": \"nonsense\"}");
            SettingsStore store (file);
            store.load();
            expectEquals (store.get().groupWindowMs, 400);
            expectEquals (store.get().mode, juce::String ("translation"));

            file.replaceWithText ("not json at all");
            store.load();
            expectEquals (store.get().groupWindowMs, 60);
        }

        directory.deleteRecursively();
    }
};

static SettingsStoreTests settingsStoreTests;
