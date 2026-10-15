#include "core/LogReader.h"
#include <algorithm>

namespace keystofrets::logreader
{
    bool parseLine (const juce::String& line, LoggedChord& out)
    {
        juce::var parsed;
        if (line.trim().isEmpty() || ! juce::JSON::parse (line, parsed).wasOk() || ! parsed.isObject())
            return false;

        const auto* notes = parsed.getProperty ("notes", juce::var()).getArray();
        if (notes == nullptr || notes->isEmpty())
            return false;

        LoggedChord chord;
        chord.timestampMs = static_cast<int> (parsed.getProperty ("t", 0));
        chord.name = parsed.getProperty ("name", juce::var()).toString();
        chord.mode = parsed.getProperty ("mode", juce::var()).toString();
        for (const auto& item : *notes)
        {
            const int midi = item.getProperty ("midi", -1);
            if (midi < 0 || midi > 127)
                continue;
            chord.notes.push_back ({ midi, juce::jlimit (1, 127, static_cast<int> (item.getProperty ("vel", 90))) });
        }
        if (chord.notes.empty())
            return false;

        out = std::move (chord);
        return true;
    }

    std::vector<LoggedChord> readChords (const juce::File& file)
    {
        std::vector<LoggedChord> chords;
        juce::StringArray lines;
        lines.addLines (file.loadFileAsString());
        for (const auto& line : lines)
        {
            LoggedChord chord;
            if (parseLine (line, chord))
                chords.push_back (std::move (chord));
        }
        return chords;
    }

    juce::String displayDateFor (const juce::File& file)
    {
        const auto name = file.getFileNameWithoutExtension();
        if (name.startsWith ("session_") && name.length() == 27)
        {
            const auto stamp = name.substring (8);
            return stamp.substring (0, 10) + " " + stamp.substring (11).replaceCharacter ('-', ':');
        }
        return file.getFileName();
    }

    std::vector<LogSummary> listSessions (const juce::File& directory)
    {
        std::vector<LogSummary> sessions;
        for (const auto& entry : juce::RangedDirectoryIterator (directory, false, "session_*.jsonl", juce::File::findFiles))
        {
            LogSummary summary;
            summary.file = entry.getFile();
            summary.displayDate = displayDateFor (summary.file);
            summary.chordCount = static_cast<int> (readChords (summary.file).size());
            sessions.push_back (std::move (summary));
        }
        std::sort (sessions.begin(), sessions.end(), [] (const LogSummary& a, const LogSummary& b)
        {
            return a.file.getFileName() > b.file.getFileName();
        });
        return sessions;
    }
}
