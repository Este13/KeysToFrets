#include "app/AppController.h"
#include "app/AppEvents.h"
#include "core/AppPaths.h"

namespace keystofrets
{
    namespace
    {
        constexpr int autosaveIntervalMs = 10000;
    }

    AppController::AppController()
        : settingsStore (paths::settingsFile()),
          sessionStartMs (juce::Time::getMillisecondCounterHiRes()),
          sessionStartTime (juce::Time::getCurrentTime()),
          detection (fifo, 60.0),
          router (engine.getMidiCollector(), fifo, detection),
          midi (engine.getDeviceManager(), router),
          replay (engine.getMidiCollector()),
          logger (paths::logsDirectory(), sessionStartTime)
    {
    }

    AppController::~AppController()
    {
        shutdown();
    }

    void AppController::start()
    {
        settingsStore.load();
        auto& settings = settingsStore.get();

        detection.setWindowMs (settings.groupWindowMs);
        detection.onChord = [weak = juce::WeakReference<AppController> (this)] (const CommittedChord& chord)
        {
            juce::MessageManager::callAsync ([weak, chord] { if (weak != nullptr) weak->handleCommittedChord (chord); });
        };
        detection.onHeldChanged = [weak = juce::WeakReference<AppController> (this)] (const std::vector<int>& notes)
        {
            juce::MessageManager::callAsync ([weak, notes] { if (weak != nullptr) weak->handleHeldNotes (notes); });
        };
        detection.start();

        const auto audioError = engine.startWithSettings (settings.audio);
        if (audioError.isEmpty())
            settings.audio = engine.currentSettings();
        engine.onChanged = [this] { pushAudioStatus(); };

        auto content = std::make_unique<UiBridge> (events::incoming(),
                                                   [this] (const juce::String& event, const juce::var& payload) { dispatch (event, payload); });
        bridge = content.get();
        window = std::make_unique<MainWindow> (std::move (content), settings.window,
                                               [] { juce::JUCEApplicationBase::getInstance()->systemRequestedQuit(); });
        bridge->load();

        midi.onChanged = [this] { rememberMidiDevice(); pushMidiStatus(); };
        midi.onDisconnected = [this] { router.resetHeldNotes(); };
        midi.start (settings.midiDeviceId);

        startTimer (autosaveIntervalMs);
        runSelfTestIfRequested();

        juce::MessageManager::callAsync ([weak = juce::WeakReference<AppController> (this)]
        {
            if (weak == nullptr)
                return;
            auto& s = weak->settingsStore.get();
            if (s.lastVstPath.isEmpty())
                return;
            const juce::File file (s.lastVstPath);
            if (file.exists())
            {
                weak->loadVst (file, true);
                return;
            }
            weak->vstNotice = "Last plugin not found: " + file.getFileName();
            s.lastVstPath = {};
            s.vstState = {};
            weak->settingsStore.save();
            weak->pushVstStatus();
        });
    }

    void AppController::shutdown()
    {
        stopTimer();
        midi.onChanged = nullptr;
        midi.onDisconnected = nullptr;
        engine.onChanged = nullptr;
        detection.onChord = nullptr;
        detection.onHeldChanged = nullptr;
        detection.stop();
        replay.stopAll();

        if (window != nullptr)
            settingsStore.get().window = window->captureState();
        persistPluginState();
        closePluginEditor();
        engine.setProcessor (nullptr);
        plugin.reset();
        settingsStore.save();
        bridge = nullptr;
        window.reset();
    }

    void AppController::timerCallback()
    {
        if (editorWindow != nullptr)
            persistPluginState();
    }

    void AppController::emit (const juce::String& event, const juce::var& payload)
    {
        if (uiReady && bridge != nullptr)
            bridge->emit (event, payload);
    }

    void AppController::showToast (const juce::String& text, bool isError)
    {
        emit (events::toast, payload::toast (text, isError));
    }

    juce::var AppController::midiStatusVar()
    {
        juce::Array<juce::var> list;
        for (const auto& device : midi.getDevices())
        {
            auto* item = new juce::DynamicObject();
            item->setProperty ("id", device.identifier);
            item->setProperty ("name", device.name);
            list.add (juce::var (item));
        }
        auto* root = new juce::DynamicObject();
        root->setProperty ("connected", midi.isConnected());
        root->setProperty ("name", midi.isConnected() ? midi.getCurrent().name : juce::String());
        root->setProperty ("id", midi.isConnected() ? midi.getCurrent().identifier : juce::String());
        root->setProperty ("devices", list);
        return juce::var (root);
    }

    juce::var AppController::vstStatusVar()
    {
        auto* root = new juce::DynamicObject();
        root->setProperty ("loaded", plugin != nullptr);
        root->setProperty ("name", pluginName);
        root->setProperty ("path", pluginFile.getFullPathName());
        root->setProperty ("notice", vstNotice);
        return juce::var (root);
    }

    juce::var AppController::buildInit()
    {
        const auto& settings = settingsStore.get();
        auto* root = new juce::DynamicObject();
        root->setProperty ("midi", midiStatusVar());
        root->setProperty ("vst", vstStatusVar());
        root->setProperty ("audio", engine.describe().toVar());
        root->setProperty ("mode", settings.mode);
        root->setProperty ("groupWindowMs", settings.groupWindowMs);
        root->setProperty ("replayLengthMs", settings.replayLengthMs);
        return juce::var (root);
    }

    void AppController::pushMidiStatus()
    {
        emit (events::midiStatus, midiStatusVar());
    }

    void AppController::pushAudioStatus()
    {
        emit (events::audioStatus, engine.describe().toVar());
    }

    void AppController::pushVstStatus()
    {
        emit (events::vstStatus, vstStatusVar());
    }

    void AppController::rememberMidiDevice()
    {
        if (! midi.isConnected())
            return;
        auto& settings = settingsStore.get();
        const auto device = midi.getCurrent();
        if (settings.midiDeviceId == device.identifier)
            return;
        settings.midiDeviceId = device.identifier;
        settings.midiDeviceName = device.name;
        settingsStore.save();
    }

    void AppController::recoverAudioAfterFailure (const juce::String& error)
    {
        engine.startWithSettings (settingsStore.get().audio);
        engine.setLastError (error);
    }

    void AppController::onSetAudio (const juce::var& payload)
    {
        const auto type = payload.getProperty ("type", juce::var()).toString();
        const auto device = payload.getProperty ("device", juce::var()).toString();
        const double rate = payload.getProperty ("sampleRate", 0.0);
        const int buffer = payload.getProperty ("bufferSize", 0);

        const auto error = engine.select (type, device, rate, buffer);
        if (error.isEmpty())
        {
            settingsStore.get().audio = engine.currentSettings();
            settingsStore.save();
        }
        else
        {
            recoverAudioAfterFailure (error);
        }
        pushAudioStatus();
    }

    void AppController::selectVst()
    {
        dialogs.chooseVst (FileDialogs::vstStartFolder (settingsStore.get().lastVstFolder),
                           [weak = juce::WeakReference<AppController> (this)] (const juce::File& chosen)
                           {
                               if (weak != nullptr)
                                   weak->vstChosen (chosen);
                           });
    }

    void AppController::vstChosen (const juce::File& chosen)
    {
        const auto file = PluginHost::normalisePath (chosen);
        settingsStore.get().lastVstFolder = file.getParentDirectory().getFullPathName();
        settingsStore.save();
        loadVst (file, false);
    }

    void AppController::loadVst (const juce::File& file, bool restoreSavedState)
    {
        closePluginEditor();
        auto result = pluginHost.load (file, engine.getSampleRate(), engine.getBufferSize());
        if (result.instance == nullptr)
        {
            vstNotice = result.error;
            pushVstStatus();
            showToast (result.error, true);
            return;
        }

        engine.setProcessor (nullptr);
        plugin = std::move (result.instance);
        pluginName = result.name;
        pluginFile = file;

        auto& settings = settingsStore.get();
        if (restoreSavedState && settings.vstState.isNotEmpty())
            PluginHost::restoreState (*plugin, settings.vstState);
        engine.setProcessor (plugin.get());

        settings.lastVstPath = file.getFullPathName();
        settings.vstState = {};
        persistPluginState();
        vstNotice = {};
        pushVstStatus();
    }

    void AppController::persistPluginState()
    {
        if (plugin == nullptr)
            return;
        auto& settings = settingsStore.get();
        const auto state = PluginHost::saveState (*plugin);
        if (state.isNotEmpty() && state != settings.vstState)
            settings.vstState = state;
        settingsStore.save();
    }

    void AppController::openPluginEditor()
    {
        if (plugin == nullptr)
            return;
        if (editorWindow != nullptr)
        {
            editorWindow->toFront (true);
            return;
        }
        if (! plugin->hasEditor())
        {
            showToast ("This plugin has no editor window.", false);
            return;
        }

        juce::AudioProcessorEditor* editor = nullptr;
        try
        {
            editor = plugin->createEditorAndMakeActive();
        }
        catch (...)
        {
            editor = nullptr;
        }
        if (editor == nullptr)
        {
            showToast ("The plugin editor could not be opened.", true);
            return;
        }

        editorWindow = std::make_unique<PluginEditorWindow> (editor, pluginName,
            [weak = juce::WeakReference<AppController> (this)]
            {
                juce::MessageManager::callAsync ([weak]
                {
                    if (weak == nullptr)
                        return;
                    weak->closePluginEditor();
                    weak->persistPluginState();
                });
            });
    }

    void AppController::closePluginEditor()
    {
        editorWindow.reset();
    }
}
