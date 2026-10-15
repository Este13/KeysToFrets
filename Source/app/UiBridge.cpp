#include "app/UiBridge.h"
#include "core/AppPaths.h"

namespace keystofrets
{
    namespace
    {
        juce::String mimeTypeFor (const juce::File& file)
        {
            const auto ext = file.getFileExtension().toLowerCase();
            if (ext == ".html") return "text/html";
            if (ext == ".js" || ext == ".mjs") return "text/javascript";
            if (ext == ".css") return "text/css";
            if (ext == ".json") return "application/json";
            if (ext == ".svg") return "image/svg+xml";
            if (ext == ".png") return "image/png";
            if (ext == ".txt") return "text/plain";
            return "application/octet-stream";
        }

        std::optional<juce::WebBrowserComponent::Resource> serve (const juce::File& root, const juce::String& url)
        {
            auto relative = url.upToFirstOccurrenceOf ("?", false, false).upToFirstOccurrenceOf ("#", false, false);
            while (relative.startsWithChar ('/'))
                relative = relative.substring (1);
            if (relative.isEmpty())
                relative = "index.html";

            const auto file = root.getChildFile (relative);
            if (! file.existsAsFile() || ! file.isAChildOf (root))
                return std::nullopt;

            juce::MemoryBlock block;
            if (! file.loadFileAsData (block))
                return std::nullopt;

            const auto* bytes = static_cast<const std::byte*> (block.getData());
            return juce::WebBrowserComponent::Resource { std::vector<std::byte> (bytes, bytes + block.getSize()), mimeTypeFor (file) };
        }
    }

    UiBridge::UiBridge (const juce::StringArray& incomingEvents, Dispatcher dispatcherToUse)
    {
        using Options = juce::WebBrowserComponent::Options;

        auto options = Options()
                           .withBackend (Options::Backend::webview2)
                           .withWinWebView2Options (Options::WinWebView2()
                                                        .withUserDataFolder (paths::webViewDataDirectory())
                                                        .withStatusBarDisabled())
                           .withNativeIntegrationEnabled()
                           .withKeepPageLoadedWhenBrowserIsHidden()
                           .withResourceProvider ([root = paths::uiRoot()] (const juce::String& url) { return serve (root, url); });

        for (const auto& eventName : incomingEvents)
        {
            options = options.withEventListener (juce::Identifier (eventName),
                                                 [dispatcherToUse, eventName] (const juce::var& payload) { dispatcherToUse (eventName, payload); });
        }

        browser = std::make_unique<juce::WebBrowserComponent> (options);
        addAndMakeVisible (*browser);
    }

    void UiBridge::load()
    {
        browser->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());
    }

    void UiBridge::emit (const juce::String& event, const juce::var& payload)
    {
        browser->emitEventIfBrowserIsVisible (juce::Identifier (event), payload);
    }

    void UiBridge::resized()
    {
        browser->setBounds (getLocalBounds());
    }
}
