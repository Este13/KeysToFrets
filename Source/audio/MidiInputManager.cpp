#include "audio/MidiInputManager.h"

namespace keystofrets
{
    namespace
    {
        constexpr int pollIntervalMs = 1000;

        const juce::MidiDeviceInfo* findById (const std::vector<juce::MidiDeviceInfo>& list, const juce::String& id)
        {
            for (const auto& device : list)
                if (device.identifier == id)
                    return &device;
            return nullptr;
        }

        bool sameDevices (const std::vector<juce::MidiDeviceInfo>& a, const std::vector<juce::MidiDeviceInfo>& b)
        {
            if (a.size() != b.size())
                return false;
            for (size_t i = 0; i < a.size(); ++i)
                if (a[i].identifier != b[i].identifier)
                    return false;
            return true;
        }
    }

    MidiInputManager::MidiInputManager (juce::AudioDeviceManager& manager, juce::MidiInputCallback& callbackToUse)
        : deviceManager (manager), callback (callbackToUse)
    {
    }

    MidiInputManager::~MidiInputManager()
    {
        stopTimer();
        closeCurrent();
    }

    void MidiInputManager::start (const juce::String& preferredIdentifier)
    {
        preferred = preferredIdentifier;
        refresh();
        startTimer (pollIntervalMs);
    }

    void MidiInputManager::selectDevice (const juce::String& identifier)
    {
        preferred = identifier;
        const auto available = juce::MidiInput::getAvailableDevices();
        devices.assign (available.begin(), available.end());
        if (const auto* target = findById (devices, identifier))
        {
            closeCurrent();
            open (*target);
        }
        if (onChanged)
            onChanged();
    }

    void MidiInputManager::closeCurrent()
    {
        if (! connected)
            return;
        deviceManager.removeMidiInputDeviceCallback (current.identifier, &callback);
        deviceManager.setMidiInputDeviceEnabled (current.identifier, false);
        connected = false;
    }

    void MidiInputManager::open (const juce::MidiDeviceInfo& device)
    {
        deviceManager.setMidiInputDeviceEnabled (device.identifier, true);
        deviceManager.addMidiInputDeviceCallback (device.identifier, &callback);
        current = device;
        connected = deviceManager.isMidiInputDeviceEnabled (device.identifier);
    }

    void MidiInputManager::refresh()
    {
        const auto available = juce::MidiInput::getAvailableDevices();
        const std::vector<juce::MidiDeviceInfo> latest (available.begin(), available.end());
        bool changed = ! sameDevices (devices, latest);
        devices = latest;

        if (connected && findById (devices, current.identifier) == nullptr)
        {
            closeCurrent();
            if (onDisconnected)
                onDisconnected();
            changed = true;
        }

        if (! connected && ! devices.empty())
        {
            const auto* target = findById (devices, preferred);
            open (target != nullptr ? *target : devices.front());
            changed = true;
        }

        if (changed && onChanged)
            onChanged();
    }

    void MidiInputManager::timerCallback()
    {
        refresh();
    }
}
