#include "app/WebView2Check.h"
#include <juce_core/juce_core.h>

namespace keystofrets
{
    namespace
    {
        constexpr auto installerPage = "https://developer.microsoft.com/microsoft-edge/webview2/#download";
        constexpr auto clientKey = "SOFTWARE\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}\\pv";

        bool hasVersion (const juce::String& path)
        {
            const auto version = juce::WindowsRegistry::getValue (path, "");
            return version.isNotEmpty() && version != "0.0.0.0";
        }
    }

    bool isWebView2RuntimeInstalled()
    {
        const juce::String key (clientKey);
        const juce::String wow ("SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}\\pv");
        return hasVersion ("HKEY_LOCAL_MACHINE\\" + wow)
            || hasVersion ("HKEY_LOCAL_MACHINE\\" + key)
            || hasVersion ("HKEY_CURRENT_USER\\" + key);
    }

    void showMissingWebView2Message (std::function<void()> onDismissed)
    {
        const auto options = juce::MessageBoxOptions()
                                 .withIconType (juce::MessageBoxIconType::WarningIcon)
                                 .withTitle ("Microsoft Edge WebView2 Runtime missing")
                                 .withMessage ("KeysToFrets needs the Microsoft Edge WebView2 Runtime to display its interface.\n\n"
                                               "Install the Evergreen runtime from:\n" + juce::String (installerPage)
                                               + "\n\nThen start KeysToFrets again.")
                                 .withButton ("Open download page")
                                 .withButton ("Quit");

        juce::AlertWindow::showAsync (options, [onDismissed] (int result)
        {
            if (result == 1)
                juce::URL (installerPage).launchInDefaultBrowser();
            onDismissed();
        });
    }
}
