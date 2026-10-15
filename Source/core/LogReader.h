#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include "core/ChordGrouper.h"

namespace keystofrets
{
    struct LoggedChord
    {
        int timestampMs = 0;
        std::vector<NoteVelocity> notes;
        juce::String name;
        juce::String mode;
    };

    struct LogSummary
    {
        juce::File file;
        juce::String displayDate;
        int chordCount = 0;
    };

    namespace logreader
    {
        std::vector<LogSummary> listSessions (const juce::File& directory);
        std::vector<LoggedChord> readChords (const juce::File& file);
        bool parseLine (const juce::String& line, LoggedChord& out);
        juce::String displayDateFor (const juce::File& file);
    }
}
