#pragma once

#include <functional>
#include <vector>
#include <juce_audio_devices/juce_audio_devices.h>

namespace keystofrets
{
    class MidiInputManager : private juce::Timer
    {
    public:
        MidiInputManager (juce::AudioDeviceManager& manager, juce::MidiInputCallback& callbackToUse);
        ~MidiInputManager() override;

        void start (const juce::String& preferredIdentifier);
        void selectDevice (const juce::String& identifier);

        bool isConnected() const noexcept { return connected; }
        juce::MidiDeviceInfo getCurrent() const { return current; }
        std::vector<juce::MidiDeviceInfo> getDevices() const { return devices; }

        std::function<void()> onChanged;
        std::function<void()> onDisconnected;

    private:
        void timerCallback() override;
        void refresh();
        void open (const juce::MidiDeviceInfo& device);
        void closeCurrent();

        juce::AudioDeviceManager& deviceManager;
        juce::MidiInputCallback& callback;
        std::vector<juce::MidiDeviceInfo> devices;
        juce::MidiDeviceInfo current;
        juce::String preferred;
        bool connected = false;
    };
}
