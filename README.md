# Chord2MIDI

A JUCE VST3 that turns a style, a key, and a chord line into MIDI you can drop
on a track. The same phrase plays in the browser at
[learningapp4-create.github.io/chord2midiVST](https://learningapp4-create.github.io/chord2midiVST/).
That page is [`index.html`](index.html) plus [`phrase.js`](phrase.js). It does
not need a build. Hear plays the phrase. Download MIDI saves a `.mid` file for
a DAW track. The VST3 itself still has to be built for the host's system.

## What you can do

Pick one of 24 styles, a key, and either that style's chord line or a generated
8-chord progression. Set tempo, feel, voicing, rhythm, a per-note envelope
(attack, decay, sustain, release), and phrase volume. Then:

- press **Hear** to preview the phrase and send note on/off from the plugin
- press play in a DAW so the plugin follows the host timeline (one chord per bar)
- drag **Drag MIDI to a track** onto a DAW track, or use **Save MIDI…**

## Build

Linux packages used here: `g++`, `cmake`, `libasound2-dev`, `libfreetype6-dev`,
`libfontconfig1-dev`, and the usual X11 dev packages (`libx11-dev`,
`libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxcomposite-dev`,
`libxext-dev`, `libxrender-dev`, `libglu1-mesa-dev`).

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/phrase_tests
```

Pin gcc and g++. A Clang `c++` that cannot find libstdc++ will not link this project.

CMake fetches JUCE 8.0.8. The VST3 is a Linux bundle under
`build/Chord2MIDI_artefacts/Release/VST3/`. The phrase tests do not need JUCE:

```bash
cmake -S . -B build-tests -DCHORD2MIDI_BUILD_PLUGIN=OFF -DCMAKE_CXX_COMPILER=g++
cmake --build build-tests
./build-tests/phrase_tests
```

This Linux build does not produce a Mac or Windows plugin. Those need the same
CMake project in Xcode or Visual Studio, plus a JUCE licence choice (AGPL or
commercial) before you ship a binary. See `THIRD_PARTY_NOTICES.md`.
