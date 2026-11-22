# Third-party notices

Chord2MIDI's own source files (everything in this repository except the fetched
JUCE tree) are under the MIT license in `LICENSE`.

## JUCE 8.0.8

The plugin is built with the [JUCE](https://juce.com) framework, tag `8.0.8`.
CMake downloads it at configure time. This repository does not vendor the JUCE
sources.

The JUCE modules are dual-licensed:

- the [GNU Affero General Public License v3](https://www.gnu.org/licenses/agpl-3.0.en.html), or
- a commercial [JUCE licence](https://juce.com/legal/juce-8-licence/) from Raw Material Software

Full terms: https://github.com/juce-framework/JUCE/blob/8.0.8/LICENSE.md

A binary of this plugin links JUCE. If you ship that binary under the AGPL,
you must follow the AGPL, including offering the corresponding source to people
you distribute the plugin to. The JUCE splash screen stays enabled in that
build. A commercial JUCE licence is the other way to ship it.

JUCE itself bundles further code (including the Steinberg VST3 SDK, which is
available under Steinberg's proprietary VST3 terms or the GPLv3). Those notices
live in the JUCE tree that CMake fetches, not in this repo.

## Musical reference

Style names, chord lines, tempos, and pattern settings follow the behavior of
the ChordCanvas beat-style catalog. The engine, UI, and MIDI writer in this
repository are original code. ChordCanvas source is not included here.
