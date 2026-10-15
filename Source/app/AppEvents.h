#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include "core/ChordGrouper.h"
#include "core/LogReader.h"

namespace keystofrets::events
{
    const juce::StringArray& incoming();

    constexpr auto init = "init";
    constexpr auto midiStatus = "midiStatus";
    constexpr auto audioStatus = "audioStatus";
    constexpr auto vstStatus = "vstStatus";
    constexpr auto held = "held";
    constexpr auto chord = "chord";
    constexpr auto logList = "logList";
    constexpr auto logOpened = "logOpened";
    constexpr auto toast = "toast";
}

namespace keystofrets::payload
{
    juce::var notes (const std::vector<NoteVelocity>& list);
    juce::var heldNotes (const std::vector<int>& list);
    juce::var committedChord (int id, int timestampMs, const CommittedChord& chord);
    juce::var logSummaries (const std::vector<LogSummary>& sessions);
    juce::var openedLog (const juce::File& file, const std::vector<LoggedChord>& chords);
    juce::var toast (const juce::String& text, bool isError);
    std::vector<NoteVelocity> parseNotes (const juce::var& notesVar);
}
