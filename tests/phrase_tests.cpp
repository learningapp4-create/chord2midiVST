#include "Types.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL " << message << "\n";
        ++failures;
    }
}

const c2m::StyleDef* findStyle(const char* name) {
    for (int i = 0; i < c2m::kStyleCount; ++i) {
        if (std::strcmp(c2m::styleAt(i).name, name) == 0)
            return &c2m::styleAt(i);
    }
    return nullptr;
}

bool samePhrase(const c2m::Phrase& a, const c2m::Phrase& b) {
    if (a.notes.size() != b.notes.size() || a.bpm != b.bpm || a.chordCount != b.chordCount)
        return false;
    for (size_t i = 0; i < a.notes.size(); ++i) {
        const auto& x = a.notes[i];
        const auto& y = b.notes[i];
        if (x.midi != y.midi || x.velocity != y.velocity) return false;
        if (std::abs(x.startBeats - y.startBeats) > 1e-9) return false;
        if (std::abs(x.lengthBeats - y.lengthBeats) > 1e-9) return false;
    }
    return true;
}

uint32_t readVarLen(const std::vector<unsigned char>& data, size_t& index) {
    uint32_t value = 0;
    for (int guard = 0; guard < 4; ++guard) {
        if (index >= data.size()) return value;
        const unsigned char byte = data[index++];
        value = (value << 7) | (byte & 0x7fu);
        if ((byte & 0x80u) == 0) break;
    }
    return value;
}

int countNoteOns(const std::vector<unsigned char>& data) {
    if (data.size() < 22) return -1;
    if (std::string(data.begin(), data.begin() + 4) != "MThd") return -1;
    size_t index = 14;
    if (std::string(data.begin() + static_cast<std::ptrdiff_t>(index), data.begin() + static_cast<std::ptrdiff_t>(index + 4)) != "MTrk")
        return -1;
    index += 8;
    int ons = 0;
    int running = 0;
    while (index < data.size()) {
        readVarLen(data, index);
        if (index >= data.size()) break;
        unsigned char status = data[index];
        if (status == 0xff) {
            ++index;
            if (index >= data.size()) break;
            const unsigned char type = data[index++];
            const uint32_t length = readVarLen(data, index);
            if (type == 0x2f) break;
            index += length;
            continue;
        }
        if (status & 0x80u) {
            running = status;
            ++index;
        }
        const int needed = ((running & 0xf0) == 0xc0 || (running & 0xf0) == 0xd0) ? 1 : 2;
        if (index + static_cast<size_t>(needed) > data.size()) break;
        if ((running & 0xf0) == 0x90 && data[index + 1] > 0) ++ons;
        index += static_cast<size_t>(needed);
    }
    return ons;
}

bool tempoMatches(const std::vector<unsigned char>& data, int bpm) {
    const uint32_t expected = static_cast<uint32_t>(std::lround(60000000.0 / bpm));
    for (size_t i = 0; i + 6 < data.size(); ++i) {
        if (data[i] == 0xff && data[i + 1] == 0x51 && data[i + 2] == 0x03) {
            const uint32_t got = (uint32_t(data[i + 3]) << 16) | (uint32_t(data[i + 4]) << 8) | data[i + 5];
            return got == expected;
        }
    }
    return false;
}

bool overlapsClean(const c2m::Phrase& phrase) {
    for (size_t i = 0; i < phrase.notes.size(); ++i) {
        for (size_t j = i + 1; j < phrase.notes.size(); ++j) {
            if (phrase.notes[j].midi != phrase.notes[i].midi) continue;
            if (phrase.notes[j].startBeats + 1e-6 < phrase.notes[i].startBeats + phrase.notes[i].lengthBeats)
                return false;
            break;
        }
    }
    return true;
}

} // namespace

int main() {
    expect(c2m::kStyleCount == 24, "24 styles");
    for (int i = 0; i < c2m::kStyleCount; ++i) {
        expect(c2m::styleAt(i).chordCount == 8, std::string(c2m::styleAt(i).name) + " has 8 chords");
        if (i > 0)
            expect(std::strcmp(c2m::styleAt(i - 1).name, c2m::styleAt(i).name) < 0, "styles are alphabetical");
        const auto chords = c2m::styleProgression(c2m::styleAt(i), 0);
        const auto phrase = c2m::renderPhrase(c2m::styleAt(i), chords, c2m::styleAt(i).defaultBpm,
                                               c2m::Feel::Straight, c2m::Density::Full, c2m::RhythmChoice::Style);
        expect(!phrase.notes.empty(), std::string(c2m::styleAt(i).name) + " renders notes");
        expect(std::abs(phrase.lengthBeats - 32.0) < 1e-9, "8 bars");
        for (const auto& note : phrase.notes) {
            expect(note.midi >= 36 && note.midi <= 96, "pitch register");
            expect(note.velocity >= 1 && note.velocity <= 127, "velocity");
        }
        expect(overlapsClean(phrase), std::string(c2m::styleAt(i).name) + " has no stuck overlaps");
        std::vector<c2m::Chord> roundtrip;
        expect(c2m::parseProgression(c2m::formatProgression(chords), roundtrip), "format parses");
        expect(roundtrip.size() == chords.size(), "roundtrip size");
    }

    const c2m::StyleDef* hope = findStyle("Hope");
    expect(hope != nullptr, "Hope exists");
    if (hope != nullptr) {
        const auto inC = c2m::styleProgression(*hope, 0);
        expect(inC[0].root == 0 && inC[0].quality == c2m::Quality::Add13, "Hope in C starts on Cadd13");
        expect(inC[2].root == 9 && inC[2].quality == c2m::Quality::Minor, "Hope in C third chord is Am");
        expect(c2m::formatProgression(c2m::styleProgression(*hope, hope->homeRoot))
                   == "Aadd13, A7sus4, F#m, Dmaj13, A, Asus2, F#m9, Dadd2",
               "Hope home spelling");

        const auto low = c2m::renderPhrase(*hope, inC, 93, c2m::Feel::Straight, c2m::Density::Full, c2m::RhythmChoice::Style);
        const auto high = c2m::renderPhrase(*hope, c2m::styleProgression(*hope, 2), 93, c2m::Feel::Straight,
                                             c2m::Density::Full, c2m::RhythmChoice::Style);
        expect(low.notes.size() == high.notes.size(), "transpose keeps the figure");
        for (size_t i = 0; i < low.notes.size(); ++i) {
            expect(high.notes[i].midi == low.notes[i].midi + 2, "key of D is two semitones above C");
            expect(std::abs(high.notes[i].startBeats - low.notes[i].startBeats) < 1e-9, "transpose keeps time");
        }

        const auto shell = c2m::renderPhrase(*hope, inC, 93, c2m::Feel::Straight, c2m::Density::Shell, c2m::RhythmChoice::Style);
        const auto full = low;
        expect(full.notes.size() > shell.notes.size(), "full voicing is denser than shell");

        c2m::StyleDef noSkip = *hope;
        noSkip.trebleSkipCount = 0;
        const auto skipped = c2m::renderPhrase(*hope, inC, 93, c2m::Feel::Straight, c2m::Density::Close, c2m::RhythmChoice::Style);
        const auto filled = c2m::renderPhrase(noSkip, inC, 93, c2m::Feel::Straight, c2m::Density::Close, c2m::RhythmChoice::Style);
        expect(filled.notes.size() > skipped.notes.size(), "rest cycle removes notes");

        const auto block = c2m::renderPhrase(*hope, inC, 93, c2m::Feel::Straight, c2m::Density::Close, c2m::RhythmChoice::Block);
        for (const auto& note : block.notes)
            expect(std::abs(note.startBeats - std::round(note.startBeats)) < 1e-6, "block chords sit on quarter beats");

        const auto loose = c2m::renderPhrase(*hope, inC, 93, c2m::Feel::Loose, c2m::Density::Full, c2m::RhythmChoice::Style);
        bool drifted = false;
        for (size_t i = 0; i < loose.notes.size() && i < full.notes.size(); ++i) {
            if (std::abs(loose.notes[i].startBeats - full.notes[i].startBeats) > 1e-6)
                drifted = true;
        }
        expect(drifted, "loose feel moves some notes off the grid");

        const auto again = c2m::renderPhrase(*hope, inC, 93, c2m::Feel::Loose, c2m::Density::Full, c2m::RhythmChoice::Style);
        expect(samePhrase(loose, again), "phrase rendering is deterministic");

        const auto slow = c2m::renderPhrase(*hope, inC, 60, c2m::Feel::Straight, c2m::Density::Full, c2m::RhythmChoice::Style);
        const auto fast = c2m::renderPhrase(*hope, inC, 120, c2m::Feel::Straight, c2m::Density::Full, c2m::RhythmChoice::Style);
        expect(slow.notes.size() == fast.notes.size(), "tempo does not change the beat grid");
        expect(std::abs(slow.notes[0].startBeats - fast.notes[0].startBeats) < 1e-9, "beat positions are tempo-independent");
        expect(tempoMatches(c2m::renderSmf(slow), 60), "MIDI tempo 60");
        expect(tempoMatches(c2m::renderSmf(fast), 120), "MIDI tempo 120");
        const auto bytes = c2m::renderSmf(full);
        expect(countNoteOns(bytes) == static_cast<int>(full.notes.size()), "MIDI note-ons match the phrase");
    }

    std::vector<c2m::Chord> parsed;
    expect(c2m::parseProgression("F#m7, Bbmaj9, C7sus4, Esus2, Adim, Caug, C11", parsed), "parser accepts the vocabulary");
    expect(parsed.size() == 7 && parsed[0].root == 6 && parsed[0].quality == c2m::Quality::Min7, "F#m7");
    expect(parsed[1].root == 10 && parsed[1].quality == c2m::Quality::Maj9, "Bbmaj9");
    expect(parsed[4].quality == c2m::Quality::Dim && parsed[6].quality == c2m::Quality::Dom11, "dim and 11");
    expect(c2m::parseProgression("Emsus2, Gsus", parsed), "sus aliases");
    expect(parsed.size() == 2 && parsed[0].quality == c2m::Quality::Sus2 && parsed[1].quality == c2m::Quality::Sus4, "msus2 and sus");
    expect(!c2m::parseProgression("H7", parsed), "reject a bad root");
    expect(!c2m::parseProgression("Cxyz", parsed), "reject an unknown quality");
    expect(!c2m::parseProgression("   ", parsed), "reject empty text");

    const auto generated = c2m::generateProgression(0, 1);
    expect(generated.size() == 8, "generated line is 8 chords");
    const int scale[] = {0, 2, 4, 5, 7, 9, 11};
    for (int i = 0; i < 4; ++i) {
        expect(generated[static_cast<size_t>(i)].root == generated[static_cast<size_t>(i + 4)].root, "second half repeats the roots");
        bool diatonic = false;
        for (int step : scale)
            if (generated[static_cast<size_t>(i)].root == step) diatonic = true;
        expect(diatonic, "generated roots stay in the key");
    }
    const auto sameSeed = c2m::generateProgression(0, 1);
    expect(c2m::formatProgression(generated) == c2m::formatProgression(sameSeed), "generator is stable for a seed");

    if (failures) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "phrase tests passed\n";
    return 0;
}
