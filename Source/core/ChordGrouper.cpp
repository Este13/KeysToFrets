#include "core/ChordGrouper.h"

namespace keystofrets
{
    void ChordGrouper::noteOn (double timeMs, int note, int velocity)
    {
        held[note] = velocity;
        group[note] = velocity;
        if (! grouping)
        {
            grouping = true;
            groupStartMs = timeMs;
        }
    }

    void ChordGrouper::noteOff (int note)
    {
        held.erase (note);
    }

    void ChordGrouper::reset()
    {
        held.clear();
        group.clear();
        grouping = false;
    }

    std::optional<CommittedChord> ChordGrouper::poll (double nowMs)
    {
        if (! grouping || nowMs < groupStartMs + windowMs)
            return std::nullopt;

        std::map<int, int> merged = held;
        for (const auto& [note, velocity] : group)
            merged[note] = velocity;

        grouping = false;
        group.clear();

        if (merged.size() < 2)
            return std::nullopt;

        CommittedChord chord;
        chord.timeMs = groupStartMs;
        for (const auto& [note, velocity] : merged)
            chord.notes.push_back ({ note, velocity });
        return chord;
    }

    std::vector<int> ChordGrouper::heldNotes() const
    {
        std::vector<int> notes;
        for (const auto& entry : held)
            notes.push_back (entry.first);
        return notes;
    }
}
