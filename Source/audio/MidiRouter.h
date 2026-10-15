#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include "core/DetectionThread.h"

namespace keystofrets
{
    class MidiRouter : public juce::MidiInputCallback
    {
    public:
        MidiRouter (juce::MidiMessageCollector& audioPathCollector, DetectionThread::Fifo& detectionFifo, DetectionThread& detectionThread)
            : collector (audioPathCollector), fifo (detectionFifo), thread (detectionThread)
        {
        }

        void handleIncomingMidiMessage (juce::MidiInput*, const juce::MidiMessage& message) override
        {
            collector.addMessageToQueue (message);

            const double now = juce::Time::getMillisecondCounterHiRes();
            if (message.isNoteOff())
                fifo.push ({ now, message.getNoteNumber(), 0, false });
            else if (message.isNoteOn())
                fifo.push ({ now, message.getNoteNumber(), message.getVelocity(), true });
            else if (message.isAllNotesOff() || message.isAllSoundOff())
                fifo.push ({ now, -1, 0, false });
            else
                return;
            thread.wake();
        }

        void resetHeldNotes()
        {
            fifo.push ({ juce::Time::getMillisecondCounterHiRes(), -1, 0, false });
            thread.wake();
        }

    private:
        juce::MidiMessageCollector& collector;
        DetectionThread::Fifo& fifo;
        DetectionThread& thread;
    };
}
