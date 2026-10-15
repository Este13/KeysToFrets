#pragma once

#include <vector>
#include <juce_core/juce_core.h>

namespace keystofrets
{
    struct DriverType
    {
        juce::String id;
        juce::String label;
    };

    struct AudioStatus
    {
        std::vector<DriverType> types;
        juce::String type;
        juce::String driverLabel;
        juce::StringArray devices;
        juce::String device;
        std::vector<double> sampleRates;
        double sampleRate = 0.0;
        std::vector<int> bufferSizes;
        int bufferSize = 0;
        double latencyMs = 0.0;
        double bufferMs = 0.0;
        int xruns = 0;
        juce::String error;

        juce::var toVar() const;
    };

    juce::String driverLabelFor (const juce::String& typeName);
    bool isSupportedDriverType (const juce::String& typeName);
}
