#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>

#include <juce_gui_extra/juce_gui_extra.h>
#include "app/AppController.h"
#include "app/WebView2Check.h"

class KeysToFretsApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "KeysToFrets"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise (const juce::String&) override
    {
        timeBeginPeriod (1);
        timerResolutionRaised = true;

        if (! keystofrets::isWebView2RuntimeInstalled())
        {
            keystofrets::showMissingWebView2Message ([this] { quit(); });
            return;
        }

        controller = std::make_unique<keystofrets::AppController>();
        controller->start();
    }

    void shutdown() override
    {
        controller.reset();
        if (timerResolutionRaised)
            timeEndPeriod (1);
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String&) override {}

private:
    std::unique_ptr<keystofrets::AppController> controller;
    bool timerResolutionRaised = false;
};

START_JUCE_APPLICATION (KeysToFretsApplication)
