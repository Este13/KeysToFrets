#include "app/MainWindow.h"

namespace keystofrets
{
    MainWindow::MainWindow (std::unique_ptr<UiBridge> content, const WindowSettings& saved, std::function<void()> onCloseRequested)
        : juce::DocumentWindow ("KeysToFrets", juce::Colour (0xff12141a), juce::DocumentWindow::allButtons),
          onClose (std::move (onCloseRequested))
    {
        setUsingNativeTitleBar (true);
        setContentOwned (content.release(), false);
        setResizable (true, false);
        setResizeLimits (720, 520, 4000, 3000);
        centreWithSize (1180, 780);
        setVisible (true);
        applySaved (saved);
    }

    void MainWindow::closeButtonPressed()
    {
        if (onClose)
            onClose();
    }

    void MainWindow::applySaved (const WindowSettings& saved)
    {
        if (! saved.valid)
            return;

        const juce::Rectangle<int> bounds (saved.x, saved.y, saved.width, saved.height);
        bool onScreen = false;
        for (const auto& display : juce::Desktop::getInstance().getDisplays().displays)
            if (display.userBounds.toNearestInt().intersects (bounds.withSizeKeepingCentre (juce::jmin (bounds.getWidth(), 200), juce::jmin (bounds.getHeight(), 100))))
                onScreen = true;
        if (! onScreen)
            return;

        const auto state = juce::String (saved.maximised ? "fs " : "") + juce::String (saved.x) + " " + juce::String (saved.y)
                         + " " + juce::String (saved.width) + " " + juce::String (saved.height);
        restoreWindowStateFromString (state);
    }

    WindowSettings MainWindow::captureState()
    {
        WindowSettings state;
        juce::StringArray tokens;
        tokens.addTokens (getWindowStateAsString(), false);
        tokens.removeEmptyStrings();
        const int first = tokens[0].startsWithIgnoreCase ("fs") ? 1 : 0;
        if (tokens.size() >= first + 4)
        {
            state.valid = true;
            state.maximised = first == 1;
            state.x = tokens[first].getIntValue();
            state.y = tokens[first + 1].getIntValue();
            state.width = tokens[first + 2].getIntValue();
            state.height = tokens[first + 3].getIntValue();
        }
        return state;
    }
}
