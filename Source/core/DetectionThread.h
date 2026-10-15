#pragma once

#include <atomic>
#include <functional>
#include <juce_core/juce_core.h>
#include "core/ChordGrouper.h"
#include "core/NoteEventFifo.h"

namespace keystofrets
{
    class DetectionThread : private juce::Thread
    {
    public:
        using Fifo = NoteEventFifo<2048>;

        DetectionThread (Fifo& source, double groupWindowMs);
        ~DetectionThread() override;

        std::function<void (const CommittedChord&)> onChord;
        std::function<void (const std::vector<int>&)> onHeldChanged;

        void start();
        void stop();
        void wake() noexcept { wakeEvent.signal(); }
        void setWindowMs (double value) noexcept { windowMs.store (value); }

    private:
        void run() override;

        Fifo& fifo;
        ChordGrouper grouper;
        std::atomic<double> windowMs;
        juce::WaitableEvent wakeEvent;
    };
}
