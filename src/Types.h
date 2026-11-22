#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace c2m {

enum class Quality {
    Major,
    Minor,
    Dom7,
    Maj7,
    Min7,
    Dom9,
    Maj9,
    Min9,
    Dom13,
    Maj13,
    Min11,
    Min13,
    Add2,
    Add4,
    Add9,
    Add13,
    Sus2,
    Sus4,
    Dom7Sus4,
    Dom9Sus4,
    Dim,
    Aug,
    Dom11
};

enum class Pattern { Block, Climb, Fall, PairsUp, PairsDown };

enum class Contour { Rise, Fall, Dip, Peak, DoublePeak, DoubleDip };

enum class Feel { Straight, Human, Loose };

enum class Density { Shell, Close, Full };

enum class RhythmChoice { Style, Block, Climb, Fall, Pairs };

struct Chord {
    int root = 0;
    Quality quality = Quality::Major;
};

struct StyleDef {
    const char* name;
    const char* character;
    int defaultBpm;
    int homeRoot;
    Chord chords[8];
    int chordCount;
    Pattern treblePattern;
    int trebleSteps;
    int trebleSkip[3];
    int trebleSkipCount;
    Contour trebleContour;
    int trebleVelocity;
    int trebleOctaves;
    Pattern bassPattern;
    int bassSteps;
    Contour bassContour;
    int bassVelocity;
    int octaveShift;
    Feel defaultFeel;
};

struct NoteEvent {
    double startBeats = 0.0;
    double lengthBeats = 0.0;
    int midi = 60;
    int velocity = 80;
};

struct Phrase {
    std::vector<NoteEvent> notes;
    double lengthBeats = 0.0;
    int bpm = 120;
    int chordCount = 0;
};

inline constexpr int kStyleCount = 24;

const char* keyName(int keyIndex);
const char* toString(Feel feel);
const char* toString(Density density);
const char* toString(RhythmChoice rhythm);

const StyleDef& styleAt(int index);

std::string formatChord(const Chord& chord);
std::string formatProgression(const std::vector<Chord>& chords);
bool parseProgression(const std::string& text, std::vector<Chord>& out);

std::vector<Chord> styleProgression(const StyleDef& style, int keyIndex);
std::vector<Chord> generateProgression(int keyIndex, std::uint32_t seed);

Phrase renderPhrase(const StyleDef& style,
                     const std::vector<Chord>& chords,
                     int bpm,
                     Feel feel,
                     Density density,
                     RhythmChoice rhythm);

std::vector<unsigned char> renderSmf(const Phrase& phrase);

} // namespace c2m
