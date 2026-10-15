#pragma once

#include <functional>
#include <memory>
#include <juce_gui_extra/juce_gui_extra.h>

namespace keystofrets
{
    class UiBridge : public juce::Component
    {
    public:
        using Dispatcher = std::function<void (const juce::String& event, const juce::var& payload)>;

        UiBridge (const juce::StringArray& incomingEvents, Dispatcher dispatcherToUse);

        void load();
        void emit (const juce::String& event, const juce::var& payload);
        void resized() override;

    private:
        std::unique_ptr<juce::WebBrowserComponent> browser;
    };
}
