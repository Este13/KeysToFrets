#include <atomic>
#include <mutex>
#include <juce_core/juce_core.h>
#include "core/DetectionThread.h"

struct DetectionThreadTests : public juce::UnitTest
{
    DetectionThreadTests() : juce::UnitTest ("DetectionThread", "KeysToFrets") {}

    void runTest() override
    {
        using namespace keystofrets;

        beginTest ("events pushed from another thread become one chord on the detection thread");
        {
            DetectionThread::Fifo fifo;
            DetectionThread thread (fifo, 60.0);
            std::atomic<int> chords { 0 };
            std::atomic<int> noteCount { 0 };
            std::atomic<bool> onDetectionThread { false };
            const auto mainThread = juce::Thread::getCurrentThreadId();

            thread.onChord = [&] (const CommittedChord& chord)
            {
                noteCount = static_cast<int> (chord.notes.size());
                onDetectionThread = juce::Thread::getCurrentThreadId() != mainThread;
                ++chords;
            };
            thread.start();

            const double now = juce::Time::getMillisecondCounterHiRes();
            fifo.push ({ now, 60, 90, true });
            fifo.push ({ now + 5.0, 64, 90, true });
            fifo.push ({ now + 9.0, 67, 90, true });
            thread.wake();

            const auto deadline = juce::Time::getMillisecondCounter() + 1000;
            while (chords.load() == 0 && juce::Time::getMillisecondCounter() < deadline)
                juce::Thread::sleep (5);

            expectEquals (chords.load(), 1);
            expectEquals (noteCount.load(), 3);
            expect (onDetectionThread.load());
            thread.stop();
        }

        beginTest ("the FIFO reports overflow instead of blocking");
        {
            DetectionThread::Fifo fifo;
            int accepted = 0;
            for (int i = 0; i < 5000; ++i)
                if (fifo.push ({ 0.0, 60, 90, true }))
                    ++accepted;
            expect (accepted < 5000);
            expect (accepted > 1000);
        }
    }
};

static DetectionThreadTests detectionThreadTests;
