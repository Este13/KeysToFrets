#pragma once

#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>

namespace keystofrets
{
    struct PluginLoadResult
    {
        std::unique_ptr<juce::AudioPluginInstance> instance;
        juce::String name;
        juce::String error;
    };

    class PluginHost
    {
    public:
        PluginHost();

        static juce::File normalisePath (const juce::File& chosen);

        PluginLoadResult load (const juce::File& file, double sampleRate, int bufferSize);

        static juce::String saveState (juce::AudioPluginInstance& instance);
        static bool restoreState (juce::AudioPluginInstance& instance, const juce::String& base64);

    private:
        juce::VST3PluginFormat format;
    };
}
