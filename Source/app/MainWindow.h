#pragma once

#include <functional>
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>
#include "app/UiBridge.h"
#include "core/SettingsStore.h"

namespace keystofrets
{
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (std::unique_ptr<UiBridge> content, const WindowSettings& saved, std::function<void()> onCloseRequested);

        void closeButtonPressed() override;
        WindowSettings captureState();

    private:
        void applySaved (const WindowSettings& saved);

        std::function<void()> onClose;
    };
}
