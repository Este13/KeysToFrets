#include "core/SessionLogger.h"

namespace keystofrets
{
    SessionLogger::SessionLogger (juce::File logsDirectory, juce::Time sessionStart)
        : directory (std::move (logsDirectory)),
          file (directory.getChildFile ("session_" + sessionStart.formatted ("%Y-%m-%d_%H-%M-%S") + ".jsonl"))
    {
    }

    juce::String SessionLogger::formatLine (int timestampMs, const std::vector<NoteVelocity>& notes, const juce::String& name, const juce::String& mode)
    {
        juce::Array<juce::var> noteList;
        for (const auto& n : notes)
        {
            auto* item = new juce::DynamicObject();
            item->setProperty ("midi", n.note);
            item->setProperty ("vel", n.velocity);
            noteList.add (juce::var (item));
        }

        auto* line = new juce::DynamicObject();
        line->setProperty ("t", timestampMs);
        line->setProperty ("notes", noteList);
        line->setProperty ("name", name);
        line->setProperty ("mode", mode);
        return juce::JSON::toString (juce::var (line), true);
    }

    bool SessionLogger::openIfNeeded()
    {
        if (stream != nullptr)
            return true;
        if (! directory.createDirectory().wasOk())
            return false;
        auto opened = std::make_unique<juce::FileOutputStream> (file);
        if (opened->failedToOpen())
            return false;
        stream = std::move (opened);
        return true;
    }

    bool SessionLogger::append (int timestampMs, const std::vector<NoteVelocity>& notes, const juce::String& name, const juce::String& mode)
    {
        if (! openIfNeeded())
            return false;
        const auto line = formatLine (timestampMs, notes, name, mode) + "\n";
        if (! stream->write (line.toRawUTF8(), line.getNumBytesAsUTF8()))
            return false;
        stream->flush();
        return true;
    }
}
