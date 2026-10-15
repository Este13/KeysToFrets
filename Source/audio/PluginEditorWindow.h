#pragma once

#include <functional>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace keystofrets
{
    class PluginEditorWindow : public juce::DocumentWindow
    {
    public:
        PluginEditorWindow (juce::AudioProcessorEditor* editor, const juce::String& title, std::function<void()> onCloseRequested);

        void closeButtonPressed() override;

    private:
        std::function<void()> onClose;
    };
}
