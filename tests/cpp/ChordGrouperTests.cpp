#include <juce_core/juce_core.h>
#include "core/ChordGrouper.h"

struct ChordGrouperTests : public juce::UnitTest
{
    ChordGrouperTests() : juce::UnitTest ("ChordGrouper", "KeysToFrets") {}

    void runTest() override
    {
        using keystofrets::ChordGrouper;

        beginTest ("a rolled chord inside the window is one chord");
        {
            ChordGrouper grouper (60.0);
            grouper.noteOn (1000.0, 60, 90);
            grouper.noteOn (1020.0, 64, 80);
            grouper.noteOn (1045.0, 67, 85);
            expect (! grouper.poll (1050.0).has_value());
            const auto chord = grouper.poll (1061.0);
            expect (chord.has_value());
            expectEquals (static_cast<int> (chord->notes.size()), 3);
            expectEquals (chord->notes.front().note, 60);
            expectEquals (chord->notes.back().note, 67);
            expectWithinAbsoluteError (chord->timeMs, 1000.0, 0.001);
            expect (! grouper.poll (2000.0).has_value());
        }

        beginTest ("a single note is never committed");
        {
            ChordGrouper grouper (60.0);
            grouper.noteOn (0.0, 60, 90);
            expect (! grouper.poll (100.0).has_value());
            expect (! grouper.isGrouping());
        }

        beginTest ("notes added later join the notes still held");
        {
            ChordGrouper grouper (60.0);
            grouper.noteOn (0.0, 60, 90);
            expect (! grouper.poll (70.0).has_value());
            grouper.noteOn (500.0, 64, 90);
            const auto chord = grouper.poll (570.0);
            expect (chord.has_value());
            expectEquals (static_cast<int> (chord->notes.size()), 2);
        }

        beginTest ("released notes are not carried into the next chord");
        {
            ChordGrouper grouper (60.0);
            grouper.noteOn (0.0, 60, 90);
            grouper.noteOn (10.0, 64, 90);
            expect (grouper.poll (80.0).has_value());
            grouper.noteOff (60);
            grouper.noteOff (64);
            expectEquals (static_cast<int> (grouper.heldNotes().size()), 0);
            grouper.noteOn (1000.0, 62, 90);
            grouper.noteOn (1010.0, 65, 90);
            const auto chord = grouper.poll (1070.0);
            expect (chord.has_value());
            expectEquals (static_cast<int> (chord->notes.size()), 2);
            expectEquals (chord->notes.front().note, 62);
        }

        beginTest ("a note released inside the window still counts for that chord");
        {
            ChordGrouper grouper (60.0);
            grouper.noteOn (0.0, 60, 90);
            grouper.noteOn (5.0, 64, 90);
            grouper.noteOff (60);
            const auto chord = grouper.poll (61.0);
            expect (chord.has_value());
            expectEquals (static_cast<int> (chord->notes.size()), 2);
        }

        beginTest ("the window is configurable");
        {
            ChordGrouper grouper (200.0);
            grouper.noteOn (0.0, 60, 90);
            grouper.noteOn (150.0, 64, 90);
            expect (! grouper.poll (190.0).has_value());
            expect (grouper.poll (201.0).has_value());
        }
    }
};

static ChordGrouperTests chordGrouperTests;
