#include "core/SettingsStore.h"

namespace keystofrets
{
    namespace
    {
        juce::String text (const juce::var& object, const char* key)
        {
            return object.getProperty (key, juce::var()).toString();
        }

        int clampedInt (const juce::var& object, const char* key, int fallback, int low, int high)
        {
            const auto value = object.getProperty (key, juce::var());
            return value.isVoid() ? fallback : juce::jlimit (low, high, static_cast<int> (value));
        }
    }

    SettingsStore::SettingsStore (juce::File fileToUse) : file (std::move (fileToUse)) {}

    juce::var SettingsStore::toVar (const Settings& value)
    {
        auto* audio = new juce::DynamicObject();
        audio->setProperty ("type", value.audio.type);
        audio->setProperty ("device", value.audio.device);
        audio->setProperty ("sampleRate", value.audio.sampleRate);
        audio->setProperty ("bufferSize", value.audio.bufferSize);

        auto* window = new juce::DynamicObject();
        window->setProperty ("x", value.window.x);
        window->setProperty ("y", value.window.y);
        window->setProperty ("width", value.window.width);
        window->setProperty ("height", value.window.height);
        window->setProperty ("maximised", value.window.maximised);

        auto* root = new juce::DynamicObject();
        root->setProperty ("lastVstPath", value.lastVstPath);
        root->setProperty ("lastVstFolder", value.lastVstFolder);
        root->setProperty ("lastLogFolder", value.lastLogFolder);
        root->setProperty ("midiDeviceId", value.midiDeviceId);
        root->setProperty ("midiDeviceName", value.midiDeviceName);
        root->setProperty ("mode", value.mode);
        root->setProperty ("groupWindowMs", value.groupWindowMs);
        root->setProperty ("replayLengthMs", value.replayLengthMs);
        root->setProperty ("audio", juce::var (audio));
        root->setProperty ("window", juce::var (window));
        root->setProperty ("vstState", value.vstState);
        return juce::var (root);
    }

    Settings SettingsStore::fromVar (const juce::var& json)
    {
        Settings result;
        if (! json.isObject())
            return result;

        result.lastVstPath = text (json, "lastVstPath");
        result.lastVstFolder = text (json, "lastVstFolder");
        result.lastLogFolder = text (json, "lastLogFolder");
        result.midiDeviceId = text (json, "midiDeviceId");
        result.midiDeviceName = text (json, "midiDeviceName");
        result.vstState = text (json, "vstState");
        result.mode = text (json, "mode") == "literal" ? "literal" : "translation";
        result.groupWindowMs = clampedInt (json, "groupWindowMs", 60, 10, 400);
        result.replayLengthMs = clampedInt (json, "replayLengthMs", 600, 50, 5000);

        const auto audio = json.getProperty ("audio", juce::var());
        if (audio.isObject())
        {
            result.audio.type = text (audio, "type");
            result.audio.device = text (audio, "device");
            result.audio.sampleRate = static_cast<double> (audio.getProperty ("sampleRate", 0.0));
            result.audio.bufferSize = clampedInt (audio, "bufferSize", 0, 0, 16384);
        }

        const auto window = json.getProperty ("window", juce::var());
        if (window.isObject() && ! window.getProperty ("width", juce::var()).isVoid())
        {
            result.window.valid = true;
            result.window.x = clampedInt (window, "x", 0, -32000, 32000);
            result.window.y = clampedInt (window, "y", 0, -32000, 32000);
            result.window.width = clampedInt (window, "width", 1180, 640, 8000);
            result.window.height = clampedInt (window, "height", 780, 480, 8000);
            result.window.maximised = static_cast<bool> (window.getProperty ("maximised", false));
        }
        return result;
    }

    void SettingsStore::load()
    {
        settings = Settings();
        if (! file.existsAsFile())
            return;
        juce::var parsed;
        if (juce::JSON::parse (file.loadFileAsString(), parsed).wasOk())
            settings = fromVar (parsed);
    }

    bool SettingsStore::save() const
    {
        file.getParentDirectory().createDirectory();
        return file.replaceWithText (juce::JSON::toString (toVar (settings), false), false, false, "\r\n");
    }
}
