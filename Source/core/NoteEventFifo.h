#pragma once

#include <array>
#include <juce_core/juce_core.h>

namespace keystofrets
{
    struct NoteEvent
    {
        double timeMs = 0.0;
        int note = 0;
        int velocity = 0;
        bool isOn = false;
    };

    template <int Capacity>
    class NoteEventFifo
    {
    public:
        bool push (const NoteEvent& event) noexcept
        {
            const auto scope = fifo.write (1);
            if (scope.blockSize1 + scope.blockSize2 < 1)
                return false;
            storage[static_cast<size_t> (scope.startIndex1)] = event;
            return true;
        }

        bool pop (NoteEvent& event) noexcept
        {
            const auto scope = fifo.read (1);
            if (scope.blockSize1 + scope.blockSize2 < 1)
                return false;
            event = storage[static_cast<size_t> (scope.startIndex1)];
            return true;
        }

    private:
        juce::AbstractFifo fifo { Capacity };
        std::array<NoteEvent, static_cast<size_t> (Capacity)> storage {};
    };
}
