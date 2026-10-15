#pragma once

#include <map>
#include <optional>
#include <vector>

namespace keystofrets
{
    struct NoteVelocity
    {
        int note = 0;
        int velocity = 0;
    };

    struct CommittedChord
    {
        double timeMs = 0.0;
        std::vector<NoteVelocity> notes;
    };

    class ChordGrouper
    {
    public:
        explicit ChordGrouper (double groupWindowMs = 60.0) : windowMs (groupWindowMs) {}

        void setWindowMs (double value) noexcept { windowMs = value; }
        double getWindowMs() const noexcept { return windowMs; }

        void noteOn (double timeMs, int note, int velocity);
        void noteOff (int note);
        void reset();
        std::optional<CommittedChord> poll (double nowMs);

        bool isGrouping() const noexcept { return grouping; }
        std::vector<int> heldNotes() const;

    private:
        std::map<int, int> held;
        std::map<int, int> group;
        double windowMs;
        double groupStartMs = 0.0;
        bool grouping = false;
    };
}
