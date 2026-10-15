#pragma once

#include <functional>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "audio/AudioStatus.h"
#include "core/SettingsStore.h"

namespace keystofrets
{
    class AudioEngine : private juce::ChangeListener
    {
    public:
        AudioEngine();
        ~AudioEngine() override;

        juce::AudioDeviceManager& getDeviceManager() noexcept { return deviceManager; }
        juce::MidiMessageCollector& getMidiCollector() noexcept { return player.getMidiMessageCollector(); }

        juce::String startWithSettings (const AudioSettings& saved);
        juce::String select (const juce::String& typeName, const juce::String& device, double sampleRate, int bufferSize);
        void setProcessor (juce::AudioProcessor* processor);
        void setLastError (const juce::String& error) { lastError = error; }

        AudioStatus describe();
        AudioSettings currentSettings();
        double getSampleRate();
        int getBufferSize();

        std::function<void()> onChanged;

    private:
        juce::AudioIODeviceType* findType (const juce::String& typeName);
        juce::String openWith (juce::AudioIODeviceType& type, const juce::String& device, double sampleRate, int bufferSize);
        void changeListenerCallback (juce::ChangeBroadcaster*) override;

        juce::AudioDeviceManager deviceManager;
        juce::AudioProcessorPlayer player;
        juce::String lastError;
    };
}
