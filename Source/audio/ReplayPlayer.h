#pragma once

#include <vector>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>
#include "core/ChordGrouper.h"

namespace keystofrets
{
    class ReplayPlayer : private juce::Timer
    {
    public:
        explicit ReplayPlayer (juce::MidiMessageCollector& collectorToUse) : collector (collectorToUse) {}
        ~ReplayPlayer() override;

        void play (const std::vector<NoteVelocity>& notes, int lengthMs);
        void stopAll();

    private:
        void timerCallback() override;
        void send (const juce::MidiMessage& message);

        juce::MidiMessageCollector& collector;
        std::vector<int> active;
    };
}
