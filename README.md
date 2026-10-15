# KeysToFrets

Windows app that listens to a USB MIDI keyboard, names the chord you play and draws it as a guitar chord diagram. The keyboard also drives a VST3 instrument live with the lowest latency WASAPI allows. Every session is logged and can be replayed through the VST.

Status: v0.1, Windows 10/11 x64.

## Features

- MIDI input with hot plug / unplug; the last device is re-opened at startup.
- VST3 hosting: file chooser, auto-reload of the last plugin and its state, editor window.
- Audio: WASAPI Exclusive (default), Shared low-latency, Shared, optional ASIO. Driver-reported output latency is displayed.
- Chord recognition with [Tonal](https://github.com/tonaljs/tonal): inversions, slash chords (`D/F#`), configurable grouping window (default 60 ms). Unrecognised sets are shown as note names.
- Two diagram modes:
  - **Translation**: [chords-db](https://github.com/tombatossals/chords-db) lookup with voicing browser ("1 of N"); falls back to a generated voicing, labelled "generated".
  - **Literal**: generator playing exactly the pressed pitch classes. Impossible cases are labelled "unplayable", with the closest voicing and the missing notes.
- Own SVG renderer, fully offline.
- One JSON Lines log per session, listed and replayed through the VST with the arrow keys.

## Architecture

C++20 + JUCE handle MIDI, audio, VST3 and logging. The UI is HTML/JS in a WebView2 `WebBrowserComponent` and never sits in the MIDI-to-VST path: MIDI goes straight to the plugin via a `MidiMessageCollector`, and chord detection is fed from a lock-free FIFO on a separate thread.

```
Source/core    settings, chord grouping, FIFO, detection thread, logs
Source/audio   audio engine, MIDI input, VST3 host, replay
Source/app     controller, WebView bridge, window, dialogs
ui             HTML/CSS/JS, bundled Tonal and chords-db data
tests          C++ (juce::UnitTest) and JS (node:test)
```

## Build

Requires Visual Studio 2022/2026 (or Build Tools) with the C++ workload (includes CMake), Git, Node.js, and the Edge WebView2 Runtime (the app explains how to get it if missing).

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

The script clones JUCE 9.0.3, downloads the WebView2 SDK, builds, runs all tests and prints the path of `KeysToFrets.exe` (in `build\KeysToFrets_artefacts\Release\`, next to its `ui` folder). Options: `-Config Debug`, `-Clean`, `-SkipTests`, `-AsioSdk <folder>`.

ASIO needs the Steinberg ASIO SDK, which is not redistributed here; without it only WASAPI is offered.

## Usage

1. Plug in the keyboard; the status pill turns green.
2. **Select VST3...** and pick a `.vst3` file (for plugins installed as folders, the file is in `Contents\x86_64-win`).
3. **Settings**: driver, device, sample rate, buffer. Increase the buffer if you hear crackles.
4. Play. Browse voicings with the arrows, the mouse wheel or Shift+Left / Shift+Right.
5. **Logs**: select a chord and press Up/Down or Left/Right to replay it.

WASAPI Exclusive silences other applications while KeysToFrets runs; use Shared low-latency to share the device.

## Data and notes

- Stored in `%APPDATA%\KeysToFrets`: `settings.json`, `logs\session_*.jsonl` (one `{"t","notes","name","mode"}` line per chord), `webview`.
- Literal mode works on pitch classes: notes outside the guitar range are shown in the nearest octave ("octave-shifted"); notes are never invented.
- Displayed latency is what the driver reports, not a loopback measurement, and excludes keyboard USB latency.
- A plugin that faults while being scanned or created is rejected without crashing the app; one that crashes later on the audio thread cannot be contained.
- `KEYSTOFRETS_SELFTEST=1` plays four demo chords through the real MIDI path shortly after start.

## Tests

```powershell
npm install
npm test
build\KeysToFretsTests_artefacts\Release\KeysToFretsTests.exe
```

## Third-party

- [JUCE](https://juce.com) 9: AGPLv3 or commercial licence
- [Tonal](https://github.com/tonaljs/tonal): MIT, bundled in `ui/vendor`
- [chords-db](https://github.com/tombatossals/chords-db): MIT, in `third_party/chords-db`
- Microsoft Edge WebView2 SDK

## License

To be decided.
