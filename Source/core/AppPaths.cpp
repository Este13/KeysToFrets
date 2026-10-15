#include "core/AppPaths.h"

namespace keystofrets::paths
{
    namespace
    {
        juce::File& rootOverride()
        {
            static juce::File value;
            return value;
        }
    }

    void overrideDataRoot (const juce::File& root)
    {
        rootOverride() = root;
    }

    juce::File dataRoot()
    {
        if (rootOverride() != juce::File())
            return rootOverride();
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("KeysToFrets");
    }

    juce::File settingsFile()
    {
        return dataRoot().getChildFile ("settings.json");
    }

    juce::File logsDirectory()
    {
        return dataRoot().getChildFile ("logs");
    }

    juce::File webViewDataDirectory()
    {
        return dataRoot().getChildFile ("webview");
    }

    juce::File uiRoot()
    {
        return juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory().getChildFile ("ui");
    }
}
