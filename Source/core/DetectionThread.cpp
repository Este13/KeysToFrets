#include "core/DetectionThread.h"

namespace keystofrets
{
    DetectionThread::DetectionThread (Fifo& source, double groupWindowMs)
        : juce::Thread ("ChordDetection"), fifo (source), grouper (groupWindowMs), windowMs (groupWindowMs)
    {
    }

    DetectionThread::~DetectionThread()
    {
        stop();
    }

    void DetectionThread::start()
    {
        startThread (juce::Thread::Priority::normal);
    }

    void DetectionThread::stop()
    {
        signalThreadShouldExit();
        wakeEvent.signal();
        stopThread (2000);
    }

    void DetectionThread::run()
    {
        while (! threadShouldExit())
        {
            grouper.setWindowMs (windowMs.load());

            bool heldChanged = false;
            NoteEvent event;
            while (fifo.pop (event))
            {
                if (event.note < 0)
                    grouper.reset();
                else if (event.isOn)
                    grouper.noteOn (event.timeMs, event.note, event.velocity);
                else
                    grouper.noteOff (event.note);
                heldChanged = true;
            }

            if (heldChanged && onHeldChanged)
                onHeldChanged (grouper.heldNotes());

            if (const auto chord = grouper.poll (juce::Time::getMillisecondCounterHiRes()))
                if (onChord)
                    onChord (*chord);

            wakeEvent.wait (grouper.isGrouping() ? 1 : 100);
        }
    }
}
