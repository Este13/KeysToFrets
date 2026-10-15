#include <juce_core/juce_core.h>
#include "core/LogReader.h"
#include "core/SessionLogger.h"

struct SessionLoggerTests : public juce::UnitTest
{
    SessionLoggerTests() : juce::UnitTest ("SessionLogger", "KeysToFrets") {}

    void runTest() override
    {
        using namespace keystofrets;
        auto directory = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("keystofrets_log_test");
        directory.deleteRecursively();
        const auto start = juce::Time (2026, 9, 9, 19, 3, 22, 0, true);

        beginTest ("no file is created when no chord is logged");
        {
            SessionLogger logger (directory, start);
            expect (! logger.hasFile());
            expect (! directory.exists());
        }

        beginTest ("each chord is one flushed JSON line and file name follows the session start");
        {
            SessionLogger logger (directory, start);
            expect (logger.append (1500, { { 60, 90 }, { 64, 80 }, { 67, 85 } }, "C", "translation"));
            expect (logger.getFile().getFileName().matchesWildcard ("session_2026-10-09_*.jsonl", false));
            expectEquals (logger.getFile().loadFileAsString().trim().length(), SessionLogger::formatLine (1500, { { 60, 90 }, { 64, 80 }, { 67, 85 } }, "C", "translation").length());
            expect (logger.append (4200, { { 62, 70 }, { 66, 70 } }, "D/F#", "literal"));

            const auto chords = logreader::readChords (logger.getFile());
            expectEquals (static_cast<int> (chords.size()), 2);
            expectEquals (chords[0].name, juce::String ("C"));
            expectEquals (chords[0].timestampMs, 1500);
            expectEquals (static_cast<int> (chords[0].notes.size()), 3);
            expectEquals (chords[0].notes[1].velocity, 80);
            expectEquals (chords[1].mode, juce::String ("literal"));
        }

        beginTest ("the reader skips malformed lines and lists sessions");
        {
            const auto extra = directory.getChildFile ("session_2025-01-02_03-04-05.jsonl");
            extra.replaceWithText ("garbage\n{\"t\":1,\"notes\":[{\"midi\":60,\"vel\":100}],\"name\":\"X\",\"mode\":\"translation\"}\n\n{\"t\":2}\n");
            const auto sessions = logreader::listSessions (directory);
            expectEquals (static_cast<int> (sessions.size()), 2);
            expect (sessions[0].file.getFileName() > sessions[1].file.getFileName());
            expectEquals (sessions[1].chordCount, 1);
            expectEquals (sessions[1].displayDate, juce::String ("2025-01-02 03:04:05"));
        }

        directory.deleteRecursively();
    }
};

static SessionLoggerTests sessionLoggerTests;
