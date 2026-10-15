#pragma once

#include <juce_core/juce_core.h>

namespace keystofrets
{
    struct AudioSettings
    {
        juce::String type;
        juce::String device;
        double sampleRate = 0.0;
        int bufferSize = 0;
    };

    struct WindowSettings
    {
        bool valid = false;
        bool maximised = false;
        int x = 0;
        int y = 0;
        int width = 1180;
        int height = 780;
    };

    struct Settings
    {
        juce::String lastVstPath;
        juce::String lastVstFolder;
        juce::String lastLogFolder;
        juce::String midiDeviceId;
        juce::String midiDeviceName;
        juce::String vstState;
        juce::String mode { "translation" };
        int groupWindowMs = 60;
        int replayLengthMs = 600;
        AudioSettings audio;
        WindowSettings window;
    };

    class SettingsStore
    {
    public:
        explicit SettingsStore (juce::File fileToUse);

        Settings& get() noexcept { return settings; }
        const Settings& get() const noexcept { return settings; }

        void load();
        bool save() const;

        static juce::var toVar (const Settings& value);
        static Settings fromVar (const juce::var& json);

    private:
        juce::File file;
        Settings settings;
    };
}
