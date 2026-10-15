#pragma once

#include <memory>
#include <vector>
#include <juce_core/juce_core.h>
#include "core/ChordGrouper.h"

namespace keystofrets
{
    class SessionLogger
    {
    public:
        SessionLogger (juce::File logsDirectory, juce::Time sessionStart);

        bool append (int timestampMs, const std::vector<NoteVelocity>& notes, const juce::String& name, const juce::String& mode);

        bool hasFile() const noexcept { return stream != nullptr; }
        juce::File getFile() const { return file; }

        static juce::String formatLine (int timestampMs, const std::vector<NoteVelocity>& notes, const juce::String& name, const juce::String& mode);

    private:
        bool openIfNeeded();

        juce::File directory;
        juce::File file;
        std::unique_ptr<juce::FileOutputStream> stream;
    };
}
