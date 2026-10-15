#pragma once

#include <juce_core/juce_core.h>

namespace keystofrets::paths
{
    juce::File dataRoot();
    juce::File settingsFile();
    juce::File logsDirectory();
    juce::File webViewDataDirectory();
    juce::File uiRoot();
    void overrideDataRoot (const juce::File& root);
}
