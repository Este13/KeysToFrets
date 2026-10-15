#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace keystofrets
{
    bool isWebView2RuntimeInstalled();
    void showMissingWebView2Message (std::function<void()> onDismissed);
}
