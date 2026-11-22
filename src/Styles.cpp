#include "Types.h"

#include <cctype>
#include <cstdlib>

namespace c2m {
namespace {

int clampKey(int key) {
    if (key < 0) return 0;
    if (key > 11) return 11;
    return key;
}

const char* pitchName(int pc) {
    static const char* names[] = {
        "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"
    };
    pc %= 12;
    if (pc < 0) pc += 12;
    return names[pc];
}

const char* qualitySuffix(Quality quality) {
    switch (quality) {
        case Quality::Major: return "";
        case Quality::Minor: return "m";
        case Quality::Dom7: return "7";
        case Quality::Maj7: return "maj7";
        case Quality::Min7: return "m7";
        case Quality::Dom9: return "9";
        case Quality::Maj9: return "maj9";
        case Quality::Min9: return "m9";
        case Quality::Dom13: return "13";
        case Quality::Maj13: return "maj13";
        case Quality::Min11: return "m11";
        case Quality::Min13: return "m13";
        case Quality::Add2: return "add2";
        case Quality::Add4: return "add4";
        case Quality::Add9: return "add9";
        case Quality::Add13: return "add13";
        case Quality::Sus2: return "sus2";
        case Quality::Sus4: return "sus4";
        case Quality::Dom7Sus4: return "7sus4";
        case Quality::Dom9Sus4: return "9sus4";
        case Quality::Dim: return "dim";
        case Quality::Aug: return "aug";
        case Quality::Dom11: return "11";
    }
    return "";
}

std::string lowerCopy(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (unsigned char ch : text)
        out.push_back(static_cast<char>(std::tolower(ch)));
    return out;
}

bool sameIgnoreCase(const std::string& text, const char* literal) {
    return lowerCopy(text) == literal;
}

bool parseQuality(const std::string& suffix, Quality& quality) {
    if (suffix.empty()) {
        quality = Quality::Major;
        return true;
    }
    if (suffix == "M7" || sameIgnoreCase(suffix, "maj7") || sameIgnoreCase(suffix, "major7")) {
        quality = Quality::Maj7;
        return true;
    }
    if (suffix == "M9" || sameIgnoreCase(suffix, "maj9") || sameIgnoreCase(suffix, "major9")) {
        quality = Quality::Maj9;
        return true;
    }
    if (suffix == "M13" || sameIgnoreCase(suffix, "maj13") || sameIgnoreCase(suffix, "major13")) {
        quality = Quality::Maj13;
        return true;
    }
    if (sameIgnoreCase(suffix, "msus2")) {
        quality = Quality::Sus2;
        return true;
    }
    if (sameIgnoreCase(suffix, "9sus4")) {
        quality = Quality::Dom9Sus4;
        return true;
    }
    if (sameIgnoreCase(suffix, "7sus4")) {
        quality = Quality::Dom7Sus4;
        return true;
    }
    if (sameIgnoreCase(suffix, "m13") || sameIgnoreCase(suffix, "min13")) {
        quality = Quality::Min13;
        return true;
    }
    if (sameIgnoreCase(suffix, "m11") || sameIgnoreCase(suffix, "min11")) {
        quality = Quality::Min11;
        return true;
    }
    if (sameIgnoreCase(suffix, "m9") || sameIgnoreCase(suffix, "min9")) {
        quality = Quality::Min9;
        return true;
    }
    if (sameIgnoreCase(suffix, "m7") || sameIgnoreCase(suffix, "min7")) {
        quality = Quality::Min7;
        return true;
    }
    if (sameIgnoreCase(suffix, "add13")) {
        quality = Quality::Add13;
        return true;
    }
    if (sameIgnoreCase(suffix, "add9")) {
        quality = Quality::Add9;
        return true;
    }
    if (sameIgnoreCase(suffix, "add4")) {
        quality = Quality::Add4;
        return true;
    }
    if (sameIgnoreCase(suffix, "add2")) {
        quality = Quality::Add2;
        return true;
    }
    if (sameIgnoreCase(suffix, "sus2")) {
        quality = Quality::Sus2;
        return true;
    }
    if (sameIgnoreCase(suffix, "sus4") || sameIgnoreCase(suffix, "sus")) {
        quality = Quality::Sus4;
        return true;
    }
    if (sameIgnoreCase(suffix, "dim")) {
        quality = Quality::Dim;
        return true;
    }
    if (sameIgnoreCase(suffix, "aug") || suffix == "+") {
        quality = Quality::Aug;
        return true;
    }
    if (sameIgnoreCase(suffix, "13")) {
        quality = Quality::Dom13;
        return true;
    }
    if (sameIgnoreCase(suffix, "11")) {
        quality = Quality::Dom11;
        return true;
    }
    if (sameIgnoreCase(suffix, "9")) {
        quality = Quality::Dom9;
        return true;
    }
    if (sameIgnoreCase(suffix, "7")) {
        quality = Quality::Dom7;
        return true;
    }
    if (sameIgnoreCase(suffix, "m") || sameIgnoreCase(suffix, "min") || suffix == "-") {
        quality = Quality::Minor;
        return true;
    }
    return false;
}

int letterPc(char letter) {
    switch (std::toupper(static_cast<unsigned char>(letter))) {
        case 'C': return 0;
        case 'D': return 2;
        case 'E': return 4;
        case 'F': return 5;
        case 'G': return 7;
        case 'A': return 9;
        case 'B': return 11;
        default: return -1;
    }
}

std::string trim(const std::string& text) {
    size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])))
        ++begin;
    size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;
    return text.substr(begin, end - begin);
}

bool parseToken(const std::string& token, Chord& chord) {
    if (token.empty()) return false;
    const int rootPc = letterPc(token[0]);
    if (rootPc < 0) return false;
    size_t index = 1;
    int pc = rootPc;
    if (index < token.size() && (token[index] == '#' || token[index] == 'b')) {
        pc = (token[index] == '#') ? (pc + 1) % 12 : (pc + 11) % 12;
        ++index;
    }
    std::string suffix = token.substr(index);
    const auto slash = suffix.find('/');
    if (slash != std::string::npos)
        suffix = suffix.substr(0, slash);
    suffix = trim(suffix);
    Quality quality = Quality::Major;
    if (!parseQuality(suffix, quality)) return false;
    chord.root = pc;
    chord.quality = quality;
    return true;
}

struct Rng {
    explicit Rng(uint32_t seed) : state(seed == 0 ? 1u : seed) {}
    uint32_t next() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
    int below(int count) { return static_cast<int>(next() % static_cast<uint32_t>(count)); }
    uint32_t state;
};

int scalePc(int key, int degree) {
    static const int steps[] = {0, 2, 4, 5, 7, 9, 11};
    return (clampKey(key) + steps[degree]) % 12;
}

Quality pickQuality(Rng& rng, int degree) {
    switch (degree) {
        case 0: {
            const Quality options[] = {Quality::Major, Quality::Major, Quality::Maj7, Quality::Maj9,
                                       Quality::Add2, Quality::Add9, Quality::Add4};
            return options[rng.below(7)];
        }
        case 1: {
            const Quality options[] = {Quality::Minor, Quality::Min7, Quality::Min9, Quality::Min11};
            return options[rng.below(4)];
        }
        case 2: {
            const Quality options[] = {Quality::Minor, Quality::Min7, Quality::Min9, Quality::Major};
            return options[rng.below(4)];
        }
        case 3: {
            const Quality options[] = {Quality::Major, Quality::Maj7, Quality::Maj9, Quality::Add9,
                                       Quality::Sus2, Quality::Minor};
            return options[rng.below(6)];
        }
        case 4: {
            const Quality options[] = {Quality::Dom7, Quality::Dom9, Quality::Dom13, Quality::Sus4,
                                       Quality::Dom7Sus4, Quality::Dom9Sus4, Quality::Major};
            return options[rng.below(7)];
        }
        default: {
            const Quality options[] = {Quality::Minor, Quality::Min7, Quality::Min9, Quality::Min11, Quality::Min13};
            return options[rng.below(5)];
        }
    }
}

// Musical settings for the 24 ChordCanvas beat styles.
// Steps are onsets per 4/4 bar. Rests are a cycle of "play N, then a rest".
static const StyleDef kStyles[] = {
#include "StyleTable.inc"
};

static_assert(sizeof(kStyles) / sizeof(StyleDef) == kStyleCount, "style catalog size");

} // namespace

const char* keyName(int keyIndex) {
    return pitchName(clampKey(keyIndex));
}

const char* toString(Feel feel) {
    switch (feel) {
        case Feel::Straight: return "Straight";
        case Feel::Human: return "Human";
        case Feel::Loose: return "Loose";
    }
    return "Straight";
}

const char* toString(Density density) {
    switch (density) {
        case Density::Shell: return "Shell";
        case Density::Close: return "Close";
        case Density::Full: return "Full";
    }
    return "Full";
}

const char* toString(RhythmChoice rhythm) {
    switch (rhythm) {
        case RhythmChoice::Style: return "Style";
        case RhythmChoice::Block: return "Block";
        case RhythmChoice::Climb: return "Climb";
        case RhythmChoice::Fall: return "Fall";
        case RhythmChoice::Pairs: return "Pairs";
    }
    return "Style";
}

const StyleDef& styleAt(int index) {
    if (index < 0) index = 0;
    if (index >= kStyleCount) index = kStyleCount - 1;
    return kStyles[index];
}

std::string formatChord(const Chord& chord) {
    return std::string(pitchName(chord.root)) + qualitySuffix(chord.quality);
}

std::string formatProgression(const std::vector<Chord>& chords) {
    std::string out;
    for (size_t i = 0; i < chords.size(); ++i) {
        if (i) out += ", ";
        out += formatChord(chords[i]);
    }
    return out;
}

bool parseProgression(const std::string& text, std::vector<Chord>& out) {
    out.clear();
    std::string token;
    auto flush = [&](bool& ok) {
        const std::string trimmed = trim(token);
        token.clear();
        if (trimmed.empty()) return;
        Chord chord;
        if (!parseToken(trimmed, chord)) ok = false;
        else out.push_back(chord);
    };
    bool ok = true;
    for (char ch : text) {
        if (ch == ',' || ch == '\n') flush(ok);
        else token.push_back(ch);
    }
    flush(ok);
    if (!ok || out.empty()) {
        out.clear();
        return false;
    }
    return true;
}

std::vector<Chord> styleProgression(const StyleDef& style, int keyIndex) {
    const int delta = (clampKey(keyIndex) - style.homeRoot + 12) % 12;
    std::vector<Chord> chords;
    const int count = style.chordCount < 0 ? 0 : (style.chordCount > 8 ? 8 : style.chordCount);
    chords.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        Chord chord = style.chords[i];
        chord.root = (chord.root + delta) % 12;
        chords.push_back(chord);
    }
    return chords;
}

std::vector<Chord> generateProgression(int keyIndex, std::uint32_t seed) {
    Rng rng(seed);
    const int bag[] = {0, 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5};
    int degrees[4];
    int previous = -1;
    for (int i = 0; i < 4; ++i) {
        int degree = bag[rng.below(12)];
        for (int guard = 0; guard < 8 && degree == previous; ++guard)
            degree = bag[rng.below(12)];
        previous = degree;
        degrees[i] = degree;
    }
    std::vector<Chord> chords;
    chords.reserve(8);
    Quality firstPass[4];
    for (int i = 0; i < 4; ++i) {
        firstPass[i] = pickQuality(rng, degrees[i]);
        chords.push_back(Chord{scalePc(keyIndex, degrees[i]), firstPass[i]});
    }
    for (int i = 0; i < 4; ++i) {
        Quality quality = pickQuality(rng, degrees[i]);
        if (quality == firstPass[i])
            quality = pickQuality(rng, degrees[i]);
        chords.push_back(Chord{scalePc(keyIndex, degrees[i]), quality});
    }
    return chords;
}

} // namespace c2m
