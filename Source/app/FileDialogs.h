#pragma once

#include <functional>
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>

namespace keystofrets
{
    class FileDialogs
    {
    public:
        using Callback = std::function<void (const juce::File&)>;

        void chooseVst (const juce::File& initialFolder, Callback onChosen);
        void chooseLog (const juce::File& initialFolder, Callback onChosen);

        static juce::File vstStartFolder (const juce::String& lastVstFolder);
        static juce::File logStartFolder (const juce::String& lastLogFolder);

    private:
        void launch (const juce::String& title, const juce::File& folder, const juce::String& wildcard, Callback onChosen);

        std::unique_ptr<juce::FileChooser> chooser;
    };
}
