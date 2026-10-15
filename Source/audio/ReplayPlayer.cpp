#include "audio/ReplayPlayer.h"

namespace keystofrets
{
    namespace
    {
        constexpr int replayChannel = 1;
    }

    ReplayPlayer::~ReplayPlayer()
    {
        stopTimer();
    }

    void ReplayPlayer::send (const juce::MidiMessage& message)
    {
        auto stamped = message;
        stamped.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
        collector.addMessageToQueue (stamped);
    }

    void ReplayPlayer::play (const std::vector<NoteVelocity>& notes, int lengthMs)
    {
        stopAll();
        for (const auto& note : notes)
        {
            const auto velocity = static_cast<juce::uint8> (juce::jlimit (1, 127, note.velocity));
            send (juce::MidiMessage::noteOn (replayChannel, note.note, velocity));
            active.push_back (note.note);
        }
        startTimer (juce::jmax (20, lengthMs));
    }

    void ReplayPlayer::stopAll()
    {
        stopTimer();
        for (const int note : active)
            send (juce::MidiMessage::noteOff (replayChannel, note));
        active.clear();
    }

    void ReplayPlayer::timerCallback()
    {
        stopAll();
    }
}
