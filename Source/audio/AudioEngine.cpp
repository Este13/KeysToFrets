#include "audio/AudioEngine.h"
#include <algorithm>

namespace keystofrets
{
    namespace
    {
        double pickSampleRate (const juce::Array<double>& available, double requested)
        {
            if (requested > 0.0 && available.contains (requested))
                return requested;
            for (const double preferred : { 48000.0, 44100.0 })
                if (available.contains (preferred))
                    return preferred;
            return available.isEmpty() ? 0.0 : available[0];
        }

        std::vector<int> bufferCandidates (const juce::Array<int>& available, int requested)
        {
            std::vector<int> sorted (available.begin(), available.end());
            std::sort (sorted.begin(), sorted.end());

            std::vector<int> candidates;
            if (requested > 0)
            {
                candidates.push_back (requested);
                for (const int size : sorted)
                    if (size > requested)
                        candidates.push_back (size);
            }
            else
            {
                candidates = sorted;
            }
            if (candidates.size() > 8)
                candidates.resize (8);
            if (candidates.empty())
                candidates.push_back (0);
            return candidates;
        }
    }

    AudioEngine::AudioEngine()
    {
        deviceManager.addChangeListener (this);
        deviceManager.addAudioCallback (&player);
    }

    AudioEngine::~AudioEngine()
    {
        deviceManager.removeChangeListener (this);
        deviceManager.removeAudioCallback (&player);
        player.setProcessor (nullptr);
        deviceManager.closeAudioDevice();
    }

    void AudioEngine::setProcessor (juce::AudioProcessor* processor)
    {
        player.setProcessor (processor);
    }

    void AudioEngine::changeListenerCallback (juce::ChangeBroadcaster*)
    {
        if (onChanged)
            onChanged();
    }

    juce::AudioIODeviceType* AudioEngine::findType (const juce::String& typeName)
    {
        for (auto* type : deviceManager.getAvailableDeviceTypes())
            if (type->getTypeName() == typeName)
                return type;
        return nullptr;
    }

    juce::String AudioEngine::openWith (juce::AudioIODeviceType& type, const juce::String& device, double sampleRate, int bufferSize)
    {
        juce::Array<double> rates;
        juce::Array<int> buffers;
        {
            std::unique_ptr<juce::AudioIODevice> probe (type.createDevice (device, {}));
            if (probe == nullptr)
                return "Could not open " + device + " with this driver.";
            rates = probe->getAvailableSampleRates();
            buffers = probe->getAvailableBufferSizes();
        }

        juce::AudioDeviceManager::AudioDeviceSetup setup;
        setup.outputDeviceName = device;
        setup.inputDeviceName = {};
        setup.useDefaultInputChannels = false;
        setup.inputChannels.clear();
        setup.useDefaultOutputChannels = false;
        setup.outputChannels.clear();
        setup.outputChannels.setRange (0, 2, true);
        setup.sampleRate = pickSampleRate (rates, sampleRate);

        juce::String error;
        for (const int candidate : bufferCandidates (buffers, bufferSize))
        {
            setup.bufferSize = candidate;
            error = deviceManager.setAudioDeviceSetup (setup, true);
            if (error.isEmpty())
                return {};
        }
        return error;
    }

    juce::String AudioEngine::select (const juce::String& typeName, const juce::String& device, double sampleRate, int bufferSize)
    {
        auto* type = findType (typeName);
        if (type == nullptr)
            return "Unknown driver type: " + typeName;

        deviceManager.setCurrentAudioDeviceType (typeName, true);
        deviceManager.closeAudioDevice();
        type->scanForDevices();

        const auto names = type->getDeviceNames (false);
        if (names.isEmpty())
            return "No output device found for " + driverLabelFor (typeName) + ".";

        auto chosen = device;
        if (! names.contains (chosen))
        {
            const int defaultIndex = type->getDefaultDeviceIndex (false);
            chosen = names[juce::isPositiveAndBelow (defaultIndex, names.size()) ? defaultIndex : 0];
        }

        lastError = openWith (*type, chosen, sampleRate, bufferSize);
        return lastError;
    }

    juce::String AudioEngine::startWithSettings (const AudioSettings& saved)
    {
        juce::StringArray order;
        if (saved.type.isNotEmpty())
            order.add (saved.type);
        for (const char* name : { "Windows Audio (Exclusive Mode)", "Windows Audio (Low Latency Mode)", "Windows Audio" })
            order.addIfNotAlreadyThere (name);

        juce::String error = "No audio driver available.";
        for (const auto& name : order)
        {
            if (findType (name) == nullptr)
                continue;
            const bool sameType = name == saved.type;
            error = select (name, sameType ? saved.device : juce::String(), sameType ? saved.sampleRate : 0.0, sameType ? saved.bufferSize : 0);
            if (error.isEmpty())
                return {};
        }
        return error;
    }

    AudioStatus AudioEngine::describe()
    {
        AudioStatus status;
        for (auto* type : deviceManager.getAvailableDeviceTypes())
            if (isSupportedDriverType (type->getTypeName()))
                status.types.push_back ({ type->getTypeName(), driverLabelFor (type->getTypeName()) });

        if (auto* type = deviceManager.getCurrentDeviceTypeObject())
        {
            status.type = type->getTypeName();
            status.driverLabel = driverLabelFor (status.type);
            status.devices = type->getDeviceNames (false);
        }

        if (auto* device = deviceManager.getCurrentAudioDevice())
        {
            status.device = deviceManager.getAudioDeviceSetup().outputDeviceName;
            for (const double rate : device->getAvailableSampleRates())
                status.sampleRates.push_back (rate);
            for (const int size : device->getAvailableBufferSizes())
                status.bufferSizes.push_back (size);
            status.sampleRate = device->getCurrentSampleRate();
            status.bufferSize = device->getCurrentBufferSizeSamples();
            if (status.sampleRate > 0.0)
            {
                status.latencyMs = device->getOutputLatencyInSamples() * 1000.0 / status.sampleRate;
                status.bufferMs = status.bufferSize * 1000.0 / status.sampleRate;
            }
            status.xruns = deviceManager.getXRunCount();
        }
        status.error = lastError;
        return status;
    }

    AudioSettings AudioEngine::currentSettings()
    {
        AudioSettings result;
        if (auto* type = deviceManager.getCurrentDeviceTypeObject())
            result.type = type->getTypeName();
        if (auto* device = deviceManager.getCurrentAudioDevice())
        {
            result.device = deviceManager.getAudioDeviceSetup().outputDeviceName;
            result.sampleRate = device->getCurrentSampleRate();
            result.bufferSize = device->getCurrentBufferSizeSamples();
        }
        return result;
    }

    double AudioEngine::getSampleRate()
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        return device != nullptr ? device->getCurrentSampleRate() : 48000.0;
    }

    int AudioEngine::getBufferSize()
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        return device != nullptr ? device->getCurrentBufferSizeSamples() : 512;
    }
}
