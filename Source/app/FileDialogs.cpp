#include "app/FileDialogs.h"
#include "core/AppPaths.h"

namespace keystofrets
{
    juce::File FileDialogs::vstStartFolder (const juce::String& lastVstFolder)
    {
        const juce::File last (lastVstFolder);
        if (lastVstFolder.isNotEmpty() && last.isDirectory())
            return last;
        const auto common = juce::File::getSpecialLocation (juce::File::globalApplicationsDirectory)
                                .getChildFile ("Common Files").getChildFile ("VST3");
        if (common.isDirectory())
            return common;
        return juce::File::getSpecialLocation (juce::File::userDesktopDirectory);
    }

    juce::File FileDialogs::logStartFolder (const juce::String& lastLogFolder)
    {
        const juce::File last (lastLogFolder);
        if (lastLogFolder.isNotEmpty() && last.isDirectory())
            return last;
        const auto logs = paths::logsDirectory();
        return logs.isDirectory() ? logs : paths::dataRoot();
    }

    void FileDialogs::launch (const juce::String& title, const juce::File& folder, const juce::String& wildcard, Callback onChosen)
    {
        chooser = std::make_unique<juce::FileChooser> (title, folder, wildcard, true);
        const auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync (flags, [onChosen = std::move (onChosen)] (const juce::FileChooser& finished)
        {
            const auto result = finished.getResult();
            if (result != juce::File() && onChosen)
                onChosen (result);
        });
    }

    void FileDialogs::chooseVst (const juce::File& initialFolder, Callback onChosen)
    {
        launch ("Select a VST3 instrument", initialFolder, "*.vst3", std::move (onChosen));
    }

    void FileDialogs::chooseLog (const juce::File& initialFolder, Callback onChosen)
    {
        launch ("Open a session log", initialFolder, "*.jsonl", std::move (onChosen));
    }
}
