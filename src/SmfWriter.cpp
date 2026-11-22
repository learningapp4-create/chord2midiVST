#include "Types.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace c2m {
namespace {

void push32(std::vector<unsigned char>& out, uint32_t value) {
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xff));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    out.push_back(static_cast<unsigned char>(value & 0xff));
}

void push16(std::vector<unsigned char>& out, uint16_t value) {
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    out.push_back(static_cast<unsigned char>(value & 0xff));
}

void pushVarLen(std::vector<unsigned char>& out, uint32_t value) {
    uint32_t buffer = value & 0x7fu;
    while ((value >>= 7) != 0) {
        buffer <<= 8;
        buffer |= (value & 0x7fu) | 0x80u;
    }
    while (true) {
        out.push_back(static_cast<unsigned char>(buffer & 0xffu));
        if (buffer & 0x80u) buffer >>= 8;
        else break;
    }
}

struct Event {
    uint32_t tick = 0;
    int order = 0;
    unsigned char status = 0;
    unsigned char data1 = 0;
    unsigned char data2 = 0;
};

} // namespace

std::vector<unsigned char> renderSmf(const Phrase& phrase) {
    constexpr int ticksPerQuarter = 960;
    const int bpm = std::clamp(phrase.bpm, 1, 999);
    const double lengthBeats = phrase.lengthBeats > 0.0 ? phrase.lengthBeats : 4.0;
    uint32_t endTick = static_cast<uint32_t>(std::llround(lengthBeats * ticksPerQuarter));

    std::vector<Event> events;
    events.reserve(phrase.notes.size() * 2);
    for (const NoteEvent& note : phrase.notes) {
        int midi = std::clamp(note.midi, 0, 127);
        int velocity = std::clamp(note.velocity, 0, 127);
        uint32_t onTick = static_cast<uint32_t>(std::llround(std::max(0.0, note.startBeats) * ticksPerQuarter));
        uint32_t offTick = static_cast<uint32_t>(std::llround(std::max(0.0, note.startBeats + note.lengthBeats) * ticksPerQuarter));
        if (offTick <= onTick) offTick = onTick + 1;
        if (velocity <= 0) continue;
        events.push_back(Event{onTick, 2, 0x90, static_cast<unsigned char>(midi), static_cast<unsigned char>(velocity)});
        events.push_back(Event{offTick, 0, 0x80, static_cast<unsigned char>(midi), 0});
        if (offTick > endTick) endTick = offTick;
    }
    for (const ExpressionEvent& expression : phrase.expression) {
        const int value = std::clamp(expression.value, 0, 127);
        uint32_t tick = static_cast<uint32_t>(std::llround(std::max(0.0, expression.beats) * ticksPerQuarter));
        events.push_back(Event{tick, 1, 0xB0, 11, static_cast<unsigned char>(value)});
        if (tick > endTick) endTick = tick;
    }
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        if (a.tick != b.tick) return a.tick < b.tick;
        return a.order < b.order;
    });

    std::vector<unsigned char> track;
    const uint32_t microseconds = static_cast<uint32_t>(std::lround(60000000.0 / bpm));
    pushVarLen(track, 0);
    track.insert(track.end(), {0xff, 0x51, 0x03,
                               static_cast<unsigned char>((microseconds >> 16) & 0xff),
                               static_cast<unsigned char>((microseconds >> 8) & 0xff),
                               static_cast<unsigned char>(microseconds & 0xff)});
    pushVarLen(track, 0);
    track.insert(track.end(), {0xff, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08});
    const char* name = "Chord2MIDI";
    pushVarLen(track, 0);
    track.insert(track.end(), {0xff, 0x03, 10});
    track.insert(track.end(), name, name + 10);

    uint32_t lastTick = 0;
    for (const Event& event : events) {
        pushVarLen(track, event.tick - lastTick);
        lastTick = event.tick;
        track.push_back(event.status);
        track.push_back(event.data1);
        track.push_back(event.data2);
    }
    pushVarLen(track, endTick >= lastTick ? endTick - lastTick : 0);
    track.insert(track.end(), {0xff, 0x2f, 0x00});

    std::vector<unsigned char> file;
    file.insert(file.end(), {'M', 'T', 'h', 'd'});
    push32(file, 6);
    push16(file, 1);
    push16(file, 1);
    push16(file, static_cast<uint16_t>(ticksPerQuarter));
    file.insert(file.end(), {'M', 'T', 'r', 'k'});
    push32(file, static_cast<uint32_t>(track.size()));
    file.insert(file.end(), track.begin(), track.end());
    return file;
}

} // namespace c2m
