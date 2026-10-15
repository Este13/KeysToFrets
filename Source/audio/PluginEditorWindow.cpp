#include "audio/PluginEditorWindow.h"

namespace keystofrets
{
    PluginEditorWindow::PluginEditorWindow (juce::AudioProcessorEditor* editor, const juce::String& title, std::function<void()> onCloseRequested)
        : juce::DocumentWindow (title, juce::Colours::black, juce::DocumentWindow::closeButton), onClose (std::move (onCloseRequested))
    {
        setUsingNativeTitleBar (true);
        setContentOwned (editor, true);
        setResizable (editor->isResizable(), false);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
        toFront (true);
    }

    void PluginEditorWindow::closeButtonPressed()
    {
        if (onClose)
            onClose();
    }
}
