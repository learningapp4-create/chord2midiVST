#include "Types.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace c2m {
namespace {

const std::vector<int>& intervalsFor(Quality quality) {
    switch (quality) {
        case Quality::Major: { static const std::vector<int> v{0, 4, 7}; return v; }
        case Quality::Minor: { static const std::vector<int> v{0, 3, 7}; return v; }
        case Quality::Dom7: { static const std::vector<int> v{0, 4, 7, 10}; return v; }
        case Quality::Maj7: { static const std::vector<int> v{0, 4, 7, 11}; return v; }
        case Quality::Min7: { static const std::vector<int> v{0, 3, 7, 10}; return v; }
        case Quality::Dom9: { static const std::vector<int> v{0, 4, 7, 10, 14}; return v; }
        case Quality::Maj9: { static const std::vector<int> v{0, 4, 7, 11, 14}; return v; }
        case Quality::Min9: { static const std::vector<int> v{0, 3, 7, 10, 14}; return v; }
        case Quality::Dom13: { static const std::vector<int> v{0, 4, 7, 10, 14, 21}; return v; }
        case Quality::Maj13: { static const std::vector<int> v{0, 4, 7, 11, 14, 21}; return v; }
        case Quality::Min11: { static const std::vector<int> v{0, 3, 7, 10, 14, 17}; return v; }
        case Quality::Min13: { static const std::vector<int> v{0, 3, 7, 10, 14, 21}; return v; }
        case Quality::Add2: { static const std::vector<int> v{0, 2, 4, 7}; return v; }
        case Quality::Add4: { static const std::vector<int> v{0, 4, 5, 7}; return v; }
        case Quality::Add9: { static const std::vector<int> v{0, 4, 7, 14}; return v; }
        case Quality::Add13: { static const std::vector<int> v{0, 4, 7, 21}; return v; }
        case Quality::Sus2: { static const std::vector<int> v{0, 2, 7}; return v; }
        case Quality::Sus4: { static const std::vector<int> v{0, 5, 7}; return v; }
        case Quality::Dom7Sus4: { static const std::vector<int> v{0, 5, 7, 10}; return v; }
        case Quality::Dom9Sus4: { static const std::vector<int> v{0, 5, 7, 10, 14}; return v; }
        case Quality::Dim: { static const std::vector<int> v{0, 3, 6}; return v; }
        case Quality::Aug: { static const std::vector<int> v{0, 4, 8}; return v; }
        case Quality::Dom11: { static const std::vector<int> v{0, 4, 7, 10, 14, 17}; return v; }
    }
    static const std::vector<int> fallback{0, 4, 7};
    return fallback;
}

int foldPitch(int note) {
    while (note > 96) note -= 12;
    while (note < 36) note += 12;
    return note;
}

std::vector<int> uniqueSorted(std::vector<int> notes) {
    std::sort(notes.begin(), notes.end());
    notes.erase(std::unique(notes.begin(), notes.end()), notes.end());
    return notes;
}

std::vector<int> shellNotes(const std::vector<int>& intervals, int rootMidi) {
    std::vector<int> notes;
    notes.push_back(rootMidi);
    int third = -1;
    int seventh = -1;
    int fifth = -1;
    int sus = -1;
    for (int interval : intervals) {
        const int pc = ((interval % 12) + 12) % 12;
        const int note = rootMidi + interval;
        if ((pc == 3 || pc == 4) && third < 0) third = note;
        else if ((pc == 10 || pc == 11) && seventh < 0) seventh = note;
        else if (pc == 7 && fifth < 0) fifth = note;
        else if ((pc == 2 || pc == 5) && sus < 0) sus = note;
    }
    if (third >= 0) notes.push_back(third);
    else if (sus >= 0) notes.push_back(sus);
    if (seventh >= 0) notes.push_back(seventh);
    else if ((int)notes.size() < 3 && fifth >= 0) notes.push_back(fifth);
    return uniqueSorted(std::move(notes));
}

struct Pools {
    std::vector<int> bass;
    std::vector<int> treble;
};

Pools voiceChord(const Chord& chord, Density density, int octaveShift, int trebleOctaves) {
    const auto& intervals = intervalsFor(chord.quality);
    const int rootMidi = 60 + (chord.root % 12);
    Pools pools;
    if (density == Density::Shell) {
        pools.treble = shellNotes(intervals, rootMidi);
    } else {
        for (int interval : intervals)
            pools.treble.push_back(rootMidi + interval);
        pools.treble = uniqueSorted(std::move(pools.treble));
        if (density == Density::Full && trebleOctaves >= 2) {
            const auto upper = pools.treble;
            for (int note : upper) {
                if (note + 12 <= 96)
                    pools.treble.push_back(note + 12);
            }
            pools.treble = uniqueSorted(std::move(pools.treble));
        }
    }
    if (density == Density::Full) {
        const int bassRoot = rootMidi - 12;
        pools.bass.push_back(bassRoot);
        for (int interval : intervals) {
            if ((((interval % 12) + 12) % 12) == 7) {
                pools.bass.push_back(bassRoot + 7);
                break;
            }
        }
    }
    const int shift = octaveShift * 12;
    for (int& note : pools.treble) note = foldPitch(note + shift);
    for (int& note : pools.bass) note = foldPitch(note + shift);
    pools.treble = uniqueSorted(std::move(pools.treble));
    pools.bass = uniqueSorted(std::move(pools.bass));
    return pools;
}

std::vector<int> pickNotes(Pattern pattern, const std::vector<int>& ordered, int cycle) {
    if (ordered.empty()) return {};
    const int count = static_cast<int>(ordered.size());
    if (pattern == Pattern::Block) return ordered;
    if (pattern == Pattern::Climb) return {ordered[cycle % count]};
    if (pattern == Pattern::Fall) return {ordered[count - 1 - (cycle % count)]};
    if (pattern == Pattern::PairsUp) {
        if (count < 2) return {ordered[0]};
        const int start = cycle % (count - 1);
        return {ordered[start], ordered[start + 1]};
    }
    if (count < 2) return {ordered[count - 1]};
    const int start = cycle % (count - 1);
    return {ordered[count - 1 - start], ordered[count - 2 - start]};
}

struct Skipper {
    const int* pattern = nullptr;
    int count = 0;
    int counter = 0;
    int index = 0;

    bool rest() {
        if (count <= 0 || pattern == nullptr) return false;
        ++counter;
        int target = pattern[index];
        if (target < 1) target = 1;
        if (counter >= target) {
            counter = 0;
            index = (index + 1) % count;
            return true;
        }
        return false;
    }
};

uint32_t mix32(uint32_t value) {
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    value ^= value >> 16;
    return value;
}

double laidBack(Feel feel, int chordIndex, int step, double stepBeats, uint32_t salt) {
    if (feel == Feel::Straight || stepBeats <= 0.0) return 0.0;
    const uint32_t hash = mix32(static_cast<uint32_t>(chordIndex) * 131u
                                 ^ static_cast<uint32_t>(step) * 17u
                                 ^ salt);
    if ((hash % 100u) >= 58u) return 0.0;
    const double amount = feel == Feel::Loose ? 0.13 : 0.055;
    const double fraction = static_cast<double>((hash >> 8) % 1000u) / 1000.0;
    return stepBeats * amount * fraction;
}

float contourGain(Contour contour, float position) {
    position = std::clamp(position, 0.0f, 1.0f);
    const auto blend = [](float from, float to, float t) { return from + (to - from) * t; };
    const auto hump = [](float t) { return t < 0.5f ? t * 2.0f : (1.0f - t) * 2.0f; };
    switch (contour) {
        case Contour::Rise: return blend(0.62f, 1.08f, position);
        case Contour::Fall: return blend(1.08f, 0.62f, position);
        case Contour::Peak: return blend(0.62f, 1.08f, hump(position));
        case Contour::Dip: return blend(1.08f, 0.62f, hump(position));
        case Contour::DoublePeak: return blend(0.66f, 1.06f, hump(std::fmod(position * 2.0f, 1.0f)));
        case Contour::DoubleDip: return blend(1.06f, 0.66f, hump(std::fmod(position * 2.0f, 1.0f)));
    }
    return 1.0f;
}

void emitPart(std::vector<NoteEvent>& notes,
              const std::vector<int>& pool,
              Pattern pattern,
              int steps,
              const int* skip,
              int skipCount,
              Contour contour,
              int baseVelocity,
              int chordIndex,
              double phraseBeats,
              Feel feel,
              uint32_t salt) {
    if (pool.empty() || steps < 1) return;
    const double stepBeats = 4.0 / static_cast<double>(steps);
    Skipper skipper{skip, skipCount, 0, 0};
    for (int step = 0; step < steps; ++step) {
        const bool silent = skipper.rest();
        if (silent) continue;
        const double at = static_cast<double>(step) * stepBeats;
        const auto chosen = pickNotes(pattern, pool, step);
        const double when = static_cast<double>(chordIndex) * 4.0 + at
                            + laidBack(feel, chordIndex, step, stepBeats, salt);
        double length = std::max(0.04, stepBeats * 0.84);
        double start = when;
        if (start < 0.0) start = 0.0;
        if (phraseBeats > 0.0 && start >= phraseBeats)
            start = std::max(0.0, phraseBeats - length);
        const float position = static_cast<float>(at / 4.0);
        int velocity = static_cast<int>(std::lround(static_cast<double>(baseVelocity) * contourGain(contour, position)));
        velocity = std::clamp(velocity, 1, 127);
        for (int midi : chosen) {
            NoteEvent event;
            event.startBeats = start;
            event.lengthBeats = length;
            event.midi = midi;
            event.velocity = velocity;
            notes.push_back(event);
        }
    }
}

void resolveOverlaps(std::vector<NoteEvent>& notes) {
    std::sort(notes.begin(), notes.end(), [](const NoteEvent& a, const NoteEvent& b) {
        if (a.startBeats != b.startBeats) return a.startBeats < b.startBeats;
        return a.midi < b.midi;
    });
    std::vector<NoteEvent> resolved;
    resolved.reserve(notes.size());
    for (const NoteEvent& event : notes) {
        bool keepLooking = true;
        while (keepLooking) {
            keepLooking = false;
            for (int i = static_cast<int>(resolved.size()) - 1; i >= 0; --i) {
                if (resolved[static_cast<size_t>(i)].midi != event.midi) continue;
                NoteEvent& previous = resolved[static_cast<size_t>(i)];
                if (previous.startBeats + previous.lengthBeats > event.startBeats + 1.0e-4) {
                    const double shortened = event.startBeats - previous.startBeats;
                    if (shortened < 0.02) resolved.erase(resolved.begin() + i);
                    else previous.lengthBeats = shortened;
                    keepLooking = true;
                }
                break;
            }
        }
        resolved.push_back(event);
    }
    notes.swap(resolved);
}

void pushRamp(std::vector<double>& times, double from, double to) {
    if (to < from) return;
    if (to <= from + 1.0e-9) {
        times.push_back(from);
        return;
    }
    constexpr int steps = 8;
    for (int i = 0; i <= steps; ++i)
        times.push_back(from + (to - from) * (static_cast<double>(i) / steps));
}

void applyDynamics(Phrase& phrase, const Envelope& requested, int requestedVolume) {
    Envelope envelope;
    envelope.attack = std::max(0.0, requested.attack);
    envelope.decay = std::max(0.0, requested.decay);
    envelope.sustain = std::clamp(requested.sustain, 0.0, 1.0);
    envelope.release = std::max(0.0, requested.release);
    const int volume = std::clamp(requestedVolume, 0, 100);
    const double gain = static_cast<double>(volume) / 100.0;
    phrase.envelope = envelope;
    phrase.volume = volume;

    for (NoteEvent& note : phrase.notes)
        note.gateBeats = note.lengthBeats;

    for (size_t i = 0; i < phrase.notes.size(); ++i) {
        NoteEvent& note = phrase.notes[i];
        double room = 1.0e9;
        for (size_t j = 0; j < phrase.notes.size(); ++j) {
            if (i == j || phrase.notes[j].midi != note.midi) continue;
            const double gap = phrase.notes[j].startBeats - (note.startBeats + note.gateBeats);
            if (gap >= -1.0e-4 && gap < room) room = gap;
        }
        double tail = envelope.release;
        if (room < 1.0e8) tail = std::min(tail, std::max(0.0, room - 1.0e-4));
        if (tail < 1.0e-4) tail = 0.0;
        note.lengthBeats = note.gateBeats + tail;
    }

    std::vector<NoteEvent> sounding;
    sounding.reserve(phrase.notes.size());
    for (NoteEvent& note : phrase.notes) {
        int velocity = static_cast<int>(std::lround(static_cast<double>(note.velocity) * gain));
        velocity = std::clamp(velocity, 0, 127);
        if (velocity <= 0) continue;
        note.velocity = velocity;
        sounding.push_back(note);
    }
    phrase.notes.swap(sounding);

    double soundingEnd = phrase.lengthBeats;
    for (const NoteEvent& note : phrase.notes)
        soundingEnd = std::max(soundingEnd, note.startBeats + note.lengthBeats);
    phrase.lengthBeats = soundingEnd;

    std::vector<double> times;
    for (const NoteEvent& note : phrase.notes) {
        const double gate = note.gateBeats;
        const double tail = std::max(0.0, note.lengthBeats - gate);
        times.push_back(note.startBeats);
        if (envelope.attack > 0.0)
            pushRamp(times, note.startBeats, note.startBeats + std::min(envelope.attack, gate));
        const double decayFrom = note.startBeats + std::min(envelope.attack, gate);
        const double decayTo = note.startBeats + std::min(envelope.attack + envelope.decay, gate);
        if (envelope.decay > 0.0 && decayTo > decayFrom)
            pushRamp(times, decayFrom, decayTo);
        times.push_back(note.startBeats + gate);
        if (tail > 0.0)
            pushRamp(times, note.startBeats + gate, note.startBeats + gate + tail);
    }
    std::sort(times.begin(), times.end());

    phrase.expression.clear();
    int previous = -1;
    double previousTime = -1.0;
    for (double time : times) {
        if (previousTime >= 0.0 && std::abs(time - previousTime) < 1.0e-6) continue;
        previousTime = time;
        double level = 0.0;
        for (const NoteEvent& note : phrase.notes) {
            const double local = time - note.startBeats;
            if (local < -1.0e-8 || local > note.lengthBeats + 1.0e-8) continue;
            level = std::max(level, envelopeAt(envelope, note.gateBeats, local));
        }
        int value = static_cast<int>(std::lround(127.0 * std::clamp(level, 0.0, 1.0) * gain));
        value = std::clamp(value, 0, 127);
        if (value == previous) continue;
        phrase.expression.push_back(ExpressionEvent{time, value});
        previous = value;
    }
}

} // namespace

double envelopeAt(const Envelope& envelope, double gateBeats, double beatsFromStart) {
    if (beatsFromStart < 0.0 || gateBeats < 0.0) return 0.0;
    const double attack = std::max(0.0, envelope.attack);
    const double decay = std::max(0.0, envelope.decay);
    const double sustain = std::clamp(envelope.sustain, 0.0, 1.0);
    const double release = std::max(0.0, envelope.release);
    if (beatsFromStart > gateBeats + release) return 0.0;

    const auto shape = [&](double time) {
        if (time < 0.0) return 0.0;
        if (attack <= 0.0 && decay <= 0.0) return sustain;
        if (attack > 0.0 && time < attack) return time / attack;
        if (decay > 0.0 && time < attack + decay) {
            const double u = (time - attack) / decay;
            return 1.0 + (sustain - 1.0) * u;
        }
        return sustain;
    };

    if (beatsFromStart < gateBeats || release <= 0.0) {
        if (release <= 0.0 && beatsFromStart >= gateBeats) return 0.0;
        return std::clamp(shape(beatsFromStart), 0.0, 1.0);
    }
    const double atRelease = std::clamp(shape(gateBeats), 0.0, 1.0);
    const double u = (beatsFromStart - gateBeats) / release;
    if (u >= 1.0) return 0.0;
    return std::clamp(atRelease * (1.0 - u), 0.0, 1.0);
}

Phrase renderPhrase(const StyleDef& style,
                     const std::vector<Chord>& chords,
                     int bpm,
                     Feel feel,
                     Density density,
                     RhythmChoice rhythm,
                     Envelope envelope,
                     int volume) {
    Phrase phrase;
    phrase.bpm = std::clamp(bpm, 40, 200);
    phrase.chordCount = static_cast<int>(chords.size());
    phrase.lengthBeats = static_cast<double>(chords.size()) * 4.0;
    if (chords.empty()) return phrase;

    Pattern treblePattern = style.treblePattern;
    int trebleSteps = std::max(1, style.trebleSteps);
    int trebleSkip[3] = {style.trebleSkip[0], style.trebleSkip[1], style.trebleSkip[2]};
    int trebleSkipCount = style.trebleSkipCount;
    Pattern bassPattern = style.bassPattern;
    int bassSteps = std::max(1, style.bassSteps);

    if (rhythm == RhythmChoice::Block) {
        treblePattern = Pattern::Block;
        bassPattern = Pattern::Block;
        trebleSteps = 4;
        bassSteps = 4;
        trebleSkipCount = 0;
    } else if (rhythm == RhythmChoice::Climb) {
        treblePattern = Pattern::Climb;
        bassPattern = Pattern::Climb;
        trebleSteps = 8;
        bassSteps = 4;
        trebleSkipCount = 0;
    } else if (rhythm == RhythmChoice::Fall) {
        treblePattern = Pattern::Fall;
        bassPattern = Pattern::Fall;
        trebleSteps = 8;
        bassSteps = 4;
        trebleSkipCount = 0;
    } else if (rhythm == RhythmChoice::Pairs) {
        treblePattern = Pattern::PairsUp;
        bassPattern = Pattern::PairsDown;
        trebleSteps = 8;
        bassSteps = 4;
        trebleSkipCount = 0;
    }

    const int octaves = density == Density::Full ? style.trebleOctaves : 1;
    for (int chordIndex = 0; chordIndex < phrase.chordCount; ++chordIndex) {
        const Pools pools = voiceChord(chords[static_cast<size_t>(chordIndex)], density, style.octaveShift, octaves);
        emitPart(phrase.notes, pools.treble, treblePattern, trebleSteps, trebleSkip, trebleSkipCount,
                 style.trebleContour, style.trebleVelocity, chordIndex, phrase.lengthBeats, feel, 0xA11u);
        emitPart(phrase.notes, pools.bass, bassPattern, bassSteps, nullptr, 0,
                 style.bassContour, style.bassVelocity, chordIndex, phrase.lengthBeats, feel, 0xB22u);
    }
    resolveOverlaps(phrase.notes);
    applyDynamics(phrase, envelope, volume);
    return phrase;
}

} // namespace c2m
