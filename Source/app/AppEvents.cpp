#include "app/AppEvents.h"

namespace keystofrets::events
{
    const juce::StringArray& incoming()
    {
        static const juce::StringArray names {
            "ready", "selectVst", "openVstEditor", "setMode", "setAudio", "setMidiDevice",
            "setSetting", "listLogs", "openLog", "browseLog", "replay", "chordResult"
        };
        return names;
    }
}

namespace keystofrets::payload
{
    juce::var notes (const std::vector<NoteVelocity>& list)
    {
        juce::Array<juce::var> items;
        for (const auto& n : list)
        {
            auto* item = new juce::DynamicObject();
            item->setProperty ("midi", n.note);
            item->setProperty ("vel", n.velocity);
            items.add (juce::var (item));
        }
        return items;
    }

    juce::var heldNotes (const std::vector<int>& list)
    {
        juce::Array<juce::var> items;
        for (const int note : list)
            items.add (note);
        auto* root = new juce::DynamicObject();
        root->setProperty ("notes", items);
        return juce::var (root);
    }

    juce::var committedChord (int id, int timestampMs, const CommittedChord& chord)
    {
        auto* root = new juce::DynamicObject();
        root->setProperty ("id", id);
        root->setProperty ("t", timestampMs);
        root->setProperty ("notes", notes (chord.notes));
        return juce::var (root);
    }

    juce::var logSummaries (const std::vector<LogSummary>& sessions)
    {
        juce::Array<juce::var> items;
        for (const auto& s : sessions)
        {
            auto* item = new juce::DynamicObject();
            item->setProperty ("path", s.file.getFullPathName());
            item->setProperty ("name", s.file.getFileName());
            item->setProperty ("date", s.displayDate);
            item->setProperty ("count", s.chordCount);
            items.add (juce::var (item));
        }
        auto* root = new juce::DynamicObject();
        root->setProperty ("logs", items);
        return juce::var (root);
    }

    juce::var openedLog (const juce::File& file, const std::vector<LoggedChord>& chords)
    {
        juce::Array<juce::var> items;
        for (const auto& c : chords)
        {
            auto* item = new juce::DynamicObject();
            item->setProperty ("t", c.timestampMs);
            item->setProperty ("notes", notes (c.notes));
            item->setProperty ("name", c.name);
            item->setProperty ("mode", c.mode);
            items.add (juce::var (item));
        }
        auto* root = new juce::DynamicObject();
        root->setProperty ("path", file.getFullPathName());
        root->setProperty ("name", logreader::displayDateFor (file));
        root->setProperty ("chords", items);
        return juce::var (root);
    }

    juce::var toast (const juce::String& text, bool isError)
    {
        auto* root = new juce::DynamicObject();
        root->setProperty ("text", text);
        root->setProperty ("kind", isError ? "bad" : "");
        return juce::var (root);
    }

    std::vector<NoteVelocity> parseNotes (const juce::var& notesVar)
    {
        std::vector<NoteVelocity> result;
        if (const auto* array = notesVar.getArray())
            for (const auto& item : *array)
            {
                const int midi = item.getProperty ("midi", -1);
                if (midi >= 0 && midi <= 127)
                    result.push_back ({ midi, juce::jlimit (1, 127, static_cast<int> (item.getProperty ("vel", 90))) });
            }
        return result;
    }
}
