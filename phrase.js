/* Chord2MIDI phrase engine for the browser demo.
   Same note times, pitches, velocities, and Standard MIDI file as src/Phrase.cpp
   and src/SmfWriter.cpp. Style rows are the values in src/StyleTable.inc. */
(function (root, factory) {
    const api = factory();
    if (typeof module !== "undefined" && module.exports) module.exports = api;
    root.C2M = api;
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
    const Quality = {
        Major: 0, Minor: 1, Dom7: 2, Maj7: 3, Min7: 4, Dom9: 5, Maj9: 6, Min9: 7,
        Dom13: 8, Maj13: 9, Min11: 10, Min13: 11, Add2: 12, Add4: 13, Add9: 14, Add13: 15,
        Sus2: 16, Sus4: 17, Dom7Sus4: 18, Dom9Sus4: 19, Dim: 20, Aug: 21, Dom11: 22
    };
    const Pattern = { Block: 0, Climb: 1, Fall: 2, PairsUp: 3, PairsDown: 4 };
    const Contour = { Rise: 0, Fall: 1, Dip: 2, Peak: 3, DoublePeak: 4, DoubleDip: 5 };
    const EveryBar = { Style: 0, None: 1, Uphill: 2, Downhill: 3, Valley: 4, Hill: 5, TwoValley: 6, TwoHill: 7 };
    const Feel = { Straight: 0, Human: 1, Loose: 2 };
    const Density = { Shell: 0, Close: 1, Full: 2 };
    const Rhythm = { Style: 0, Block: 1, Climb: 2, Fall: 3, Pairs: 4 };
    const PITCHES = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"];
    const SUFFIX = ["", "m", "7", "maj7", "m7", "9", "maj9", "m9", "13", "maj13", "m11", "m13",
        "add2", "add4", "add9", "add13", "sus2", "sus4", "7sus4", "9sus4", "dim", "aug", "11"];
    const INTERVALS = [
        [0, 4, 7], [0, 3, 7], [0, 4, 7, 10], [0, 4, 7, 11], [0, 3, 7, 10],
        [0, 4, 7, 10, 14], [0, 4, 7, 11, 14], [0, 3, 7, 10, 14],
        [0, 4, 7, 10, 14, 21], [0, 4, 7, 11, 14, 21], [0, 3, 7, 10, 14, 17], [0, 3, 7, 10, 14, 21],
        [0, 2, 4, 7], [0, 4, 5, 7], [0, 4, 7, 14], [0, 4, 7, 21],
        [0, 2, 7], [0, 5, 7], [0, 5, 7, 10], [0, 5, 7, 10, 14],
        [0, 3, 6], [0, 4, 8], [0, 4, 7, 10, 14, 17]
    ];
    const SCALE = [0, 2, 4, 5, 7, 9, 11];
    const QUALITY_BAG = [
        [Quality.Major, Quality.Major, Quality.Maj7, Quality.Maj9, Quality.Add2, Quality.Add9, Quality.Add4],
        [Quality.Minor, Quality.Min7, Quality.Min9, Quality.Min11],
        [Quality.Minor, Quality.Min7, Quality.Min9, Quality.Major],
        [Quality.Major, Quality.Maj7, Quality.Maj9, Quality.Add9, Quality.Sus2, Quality.Minor],
        [Quality.Dom7, Quality.Dom9, Quality.Dom13, Quality.Sus4, Quality.Dom7Sus4, Quality.Dom9Sus4, Quality.Major],
        [Quality.Minor, Quality.Min7, Quality.Min9, Quality.Min11, Quality.Min13]
    ];

    const STYLES = [
{
        name: "Await",
        character: "Climbing pairs (sixteenth notes) over a falling bass (eighth notes), human timing, wide treble, with rests.",
        bpm: 71,
        home: 7,
        chords: [{root:7, quality:Quality.Dom7}, {root:3, quality:Quality.Maj9}, {root:8, quality:Quality.Maj9}, {root:0, quality:Quality.Dom9Sus4}, {root:7, quality:Quality.Dom7}, {root:3, quality:Quality.Add4}, {root:8, quality:Quality.Add9}, {root:0, quality:Quality.Dom9Sus4}],
        chordCount: 8,
        treblePattern: Pattern.PairsUp,
        trebleSteps: 16,
        trebleSkip: [5, 2, 7],
        trebleSkipCount: 3,
        trebleContour: Contour.Rise,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Fall,
        bassSteps: 8,
        bassContour: Contour.DoublePeak,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Determine",
        character: "Falling pairs (eighth notes) over falling bass pairs (sixteenth notes), loose timing, with rests.",
        bpm: 113,
        home: 3,
        chords: [{root:3, quality:Quality.Maj7}, {root:7, quality:Quality.Min11}, {root:5, quality:Quality.Dom7Sus4}, {root:7, quality:Quality.Min13}, {root:3, quality:Quality.Maj9}, {root:7, quality:Quality.Min9}, {root:5, quality:Quality.Sus4}, {root:7, quality:Quality.Min13}],
        chordCount: 8,
        treblePattern: Pattern.PairsDown,
        trebleSteps: 8,
        trebleSkip: [8, 8, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Fall,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.PairsDown,
        bassSteps: 16,
        bassContour: Contour.Peak,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Elephant",
        character: "A rising line (quarter notes) over falling bass pairs (quarter notes), straight timing, wide treble, with rests.",
        bpm: 102,
        home: 4,
        chords: [{root:4, quality:Quality.Min11}, {root:2, quality:Quality.Maj13}, {root:4, quality:Quality.Min9}, {root:11, quality:Quality.Minor}, {root:4, quality:Quality.Minor}, {root:2, quality:Quality.Major}, {root:4, quality:Quality.Min9}, {root:11, quality:Quality.Min9}],
        chordCount: 8,
        treblePattern: Pattern.Climb,
        trebleSteps: 4,
        trebleSkip: [7, 0, 0],
        trebleSkipCount: 1,
        trebleContour: Contour.Rise,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.PairsDown,
        bassSteps: 4,
        bassContour: Contour.Peak,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Floating",
        character: "Climbing pairs (sixteenth notes) over falling bass pairs (sixteenth notes), human timing, with rests.",
        bpm: 95,
        home: 10,
        chords: [{root:10, quality:Quality.Add9}, {root:7, quality:Quality.Minor}, {root:5, quality:Quality.Dom9Sus4}, {root:5, quality:Quality.Add13}, {root:10, quality:Quality.Add4}, {root:7, quality:Quality.Minor}, {root:5, quality:Quality.Dom13}, {root:5, quality:Quality.Sus2}],
        chordCount: 8,
        treblePattern: Pattern.PairsUp,
        trebleSteps: 16,
        trebleSkip: [5, 6, 5],
        trebleSkipCount: 3,
        trebleContour: Contour.Peak,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.PairsDown,
        bassSteps: 16,
        bassContour: Contour.Rise,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Flowy",
        character: "A rising line (eighth-note triplets) over a rising bass (whole notes), loose timing, wide treble, with rests.",
        bpm: 135,
        home: 3,
        chords: [{root:3, quality:Quality.Minor}, {root:4, quality:Quality.Maj7}, {root:1, quality:Quality.Min9}, {root:11, quality:Quality.Add9}, {root:3, quality:Quality.Min9}, {root:4, quality:Quality.Major}, {root:1, quality:Quality.Min13}, {root:11, quality:Quality.Add4}],
        chordCount: 8,
        treblePattern: Pattern.Climb,
        trebleSteps: 12,
        trebleSkip: [4, 3, 8],
        trebleSkipCount: 3,
        trebleContour: Contour.Peak,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Climb,
        bassSteps: 1,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Giant",
        character: "Climbing pairs (eighth-note triplets) over a block bass (quarter notes), straight timing, with rests.",
        bpm: 80,
        home: 10,
        chords: [{root:10, quality:Quality.Min13}, {root:1, quality:Quality.Minor}, {root:1, quality:Quality.Min9}, {root:10, quality:Quality.Min11}, {root:10, quality:Quality.Min9}, {root:1, quality:Quality.Min11}, {root:1, quality:Quality.Min7}, {root:10, quality:Quality.Min13}],
        chordCount: 8,
        treblePattern: Pattern.PairsUp,
        trebleSteps: 12,
        trebleSkip: [7, 7, 3],
        trebleSkipCount: 3,
        trebleContour: Contour.Dip,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Block,
        bassSteps: 4,
        bassContour: Contour.DoubleDip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Heartfelt",
        character: "Falling pairs (eighth notes) over a block bass (quarter notes), human timing, with rests.",
        bpm: 120,
        home: 2,
        chords: [{root:2, quality:Quality.Min11}, {root:9, quality:Quality.Min11}, {root:4, quality:Quality.Sus2}, {root:7, quality:Quality.Sus4}, {root:5, quality:Quality.Sus2}, {root:9, quality:Quality.Min7}, {root:4, quality:Quality.Min7}, {root:4, quality:Quality.Min11}],
        chordCount: 8,
        treblePattern: Pattern.PairsDown,
        trebleSteps: 8,
        trebleSkip: [2, 8, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Peak,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Block,
        bassSteps: 4,
        bassContour: Contour.DoubleDip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Heaven",
        character: "Climbing pairs (quarter notes) over a rising bass (half notes), human timing, wide treble, up an octave, with rests.",
        bpm: 111,
        home: 11,
        chords: [{root:11, quality:Quality.Min13}, {root:7, quality:Quality.Major}, {root:9, quality:Quality.Dom7Sus4}, {root:2, quality:Quality.Maj7}, {root:11, quality:Quality.Minor}, {root:7, quality:Quality.Maj13}, {root:9, quality:Quality.Dom13}, {root:2, quality:Quality.Maj9}],
        chordCount: 8,
        treblePattern: Pattern.PairsUp,
        trebleSteps: 4,
        trebleSkip: [7, 4, 2],
        trebleSkipCount: 3,
        trebleContour: Contour.Peak,
        trebleVelocity: 66,
        trebleOctaves: 2,
        bassPattern: Pattern.Climb,
        bassSteps: 2,
        bassContour: Contour.Fall,
        bassVelocity: 77,
        octaveShift: 1,
        feel: Feel.Human
    },
{
        name: "Hesitation",
        character: "Falling pairs (quarter notes) over a rising bass (sixteenth notes), loose timing, with rests.",
        bpm: 78,
        home: 10,
        chords: [{root:10, quality:Quality.Min7}, {root:0, quality:Quality.Dom13}, {root:10, quality:Quality.Min11}, {root:10, quality:Quality.Add2}, {root:10, quality:Quality.Min11}, {root:0, quality:Quality.Maj7}, {root:10, quality:Quality.Min13}, {root:10, quality:Quality.Maj13}],
        chordCount: 8,
        treblePattern: Pattern.PairsDown,
        trebleSteps: 4,
        trebleSkip: [4, 8, 8],
        trebleSkipCount: 3,
        trebleContour: Contour.Fall,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Climb,
        bassSteps: 16,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Hollow",
        character: "Block chords (half notes) over a rising bass (quarter-note triplets), human timing, wide treble, with rests.",
        bpm: 88,
        home: 8,
        chords: [{root:8, quality:Quality.Minor}, {root:8, quality:Quality.Min7}, {root:10, quality:Quality.Minor}, {root:3, quality:Quality.Dom7Sus4}, {root:8, quality:Quality.Minor}, {root:8, quality:Quality.Min9}, {root:10, quality:Quality.Min9}, {root:3, quality:Quality.Dom9Sus4}],
        chordCount: 8,
        treblePattern: Pattern.Block,
        trebleSteps: 2,
        trebleSkip: [2, 8, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Dip,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Climb,
        bassSteps: 6,
        bassContour: Contour.DoubleDip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Hope",
        character: "Climbing pairs (half notes) over a falling bass (eighth-note triplets), loose timing, wide treble, with rests.",
        bpm: 93,
        home: 9,
        chords: [{root:9, quality:Quality.Add13}, {root:9, quality:Quality.Dom7Sus4}, {root:6, quality:Quality.Minor}, {root:2, quality:Quality.Maj13}, {root:9, quality:Quality.Major}, {root:9, quality:Quality.Sus2}, {root:6, quality:Quality.Min9}, {root:2, quality:Quality.Add2}],
        chordCount: 8,
        treblePattern: Pattern.PairsUp,
        trebleSteps: 2,
        trebleSkip: [2, 2, 4],
        trebleSkipCount: 3,
        trebleContour: Contour.Dip,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Fall,
        bassSteps: 12,
        bassContour: Contour.Rise,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Horse",
        character: "A falling line (eighth-note triplets) over falling bass pairs (half notes), loose timing, with rests.",
        bpm: 124,
        home: 5,
        chords: [{root:5, quality:Quality.Minor}, {root:8, quality:Quality.Maj7}, {root:7, quality:Quality.Major}, {root:10, quality:Quality.Major}, {root:5, quality:Quality.Min11}, {root:8, quality:Quality.Major}, {root:7, quality:Quality.Dom9}, {root:10, quality:Quality.Dom9Sus4}],
        chordCount: 8,
        treblePattern: Pattern.Fall,
        trebleSteps: 12,
        trebleSkip: [3, 2, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.DoublePeak,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.PairsDown,
        bassSteps: 2,
        bassContour: Contour.Rise,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Night",
        character: "A falling line (quarter notes) over a block bass (half notes), loose timing, with rests.",
        bpm: 118,
        home: 8,
        chords: [{root:8, quality:Quality.Min13}, {root:9, quality:Quality.Maj9}, {root:8, quality:Quality.Min11}, {root:4, quality:Quality.Add2}, {root:8, quality:Quality.Min9}, {root:9, quality:Quality.Maj9}, {root:8, quality:Quality.Minor}, {root:4, quality:Quality.Add4}],
        chordCount: 8,
        treblePattern: Pattern.Fall,
        trebleSteps: 4,
        trebleSkip: [3, 7, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Dip,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Block,
        bassSteps: 2,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Pacing",
        character: "Block chords (eighth-note triplets) over a rising bass (eighth-note triplets), human timing, wide treble, with rests.",
        bpm: 116,
        home: 5,
        chords: [{root:5, quality:Quality.Sus4}, {root:7, quality:Quality.Min11}, {root:7, quality:Quality.Min11}, {root:3, quality:Quality.Major}, {root:5, quality:Quality.Maj7}, {root:7, quality:Quality.Min13}, {root:7, quality:Quality.Min11}, {root:3, quality:Quality.Add9}],
        chordCount: 8,
        treblePattern: Pattern.Block,
        trebleSteps: 12,
        trebleSkip: [3, 5, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Dip,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Climb,
        bassSteps: 12,
        bassContour: Contour.Rise,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Purpose",
        character: "Block chords (quarter notes) over a rising bass (whole notes), straight timing, with rests.",
        bpm: 79,
        home: 3,
        chords: [{root:3, quality:Quality.Min11}, {root:5, quality:Quality.Min11}, {root:6, quality:Quality.Maj9}, {root:1, quality:Quality.Major}, {root:3, quality:Quality.Min13}, {root:5, quality:Quality.Min9}, {root:6, quality:Quality.Maj13}, {root:1, quality:Quality.Maj7}],
        chordCount: 8,
        treblePattern: Pattern.Block,
        trebleSteps: 4,
        trebleSkip: [8, 3, 3],
        trebleSkipCount: 3,
        trebleContour: Contour.Fall,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Climb,
        bassSteps: 1,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Rationale",
        character: "A rising line (quarter notes) over a falling bass (quarter notes), loose timing, wide treble, with rests.",
        bpm: 74,
        home: 3,
        chords: [{root:3, quality:Quality.Dom7}, {root:11, quality:Quality.Maj7}, {root:8, quality:Quality.Dom9Sus4}, {root:4, quality:Quality.Min9}, {root:3, quality:Quality.Dom9}, {root:11, quality:Quality.Add2}, {root:8, quality:Quality.Dom7Sus4}, {root:4, quality:Quality.Min7}],
        chordCount: 8,
        treblePattern: Pattern.Climb,
        trebleSteps: 4,
        trebleSkip: [3, 2, 6],
        trebleSkipCount: 3,
        trebleContour: Contour.DoublePeak,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Fall,
        bassSteps: 4,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    },
{
        name: "Relieve",
        character: "Falling pairs (quarter-note triplets) over a block bass (whole notes), human timing, wide treble, with rests.",
        bpm: 120,
        home: 2,
        chords: [{root:2, quality:Quality.Min11}, {root:9, quality:Quality.Min11}, {root:4, quality:Quality.Sus2}, {root:7, quality:Quality.Sus4}, {root:5, quality:Quality.Sus2}, {root:9, quality:Quality.Min7}, {root:4, quality:Quality.Min7}, {root:4, quality:Quality.Min11}],
        chordCount: 8,
        treblePattern: Pattern.PairsDown,
        trebleSteps: 6,
        trebleSkip: [2, 8, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Peak,
        trebleVelocity: 80,
        trebleOctaves: 2,
        bassPattern: Pattern.Block,
        bassSteps: 1,
        bassContour: Contour.Dip,
        bassVelocity: 51,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Romance",
        character: "A falling line (eighth-note triplets) over rising bass pairs (quarter notes), straight timing, wide treble, with rests.",
        bpm: 75,
        home: 9,
        chords: [{root:9, quality:Quality.Maj7}, {root:6, quality:Quality.Min9}, {root:2, quality:Quality.Maj7}, {root:9, quality:Quality.Maj9}, {root:9, quality:Quality.Major}, {root:6, quality:Quality.Minor}, {root:2, quality:Quality.Maj13}, {root:9, quality:Quality.Major}],
        chordCount: 8,
        treblePattern: Pattern.Fall,
        trebleSteps: 12,
        trebleSkip: [7, 5, 8],
        trebleSkipCount: 3,
        trebleContour: Contour.Fall,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.PairsUp,
        bassSteps: 4,
        bassContour: Contour.Peak,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Slickback",
        character: "Block chords (sixteenth notes) over rising bass pairs (quarter notes), loose timing, wide treble, up an octave, with rests.",
        bpm: 135,
        home: 3,
        chords: [{root:3, quality:Quality.Min11}, {root:1, quality:Quality.Maj9}, {root:6, quality:Quality.Maj13}, {root:11, quality:Quality.Major}, {root:3, quality:Quality.Min11}, {root:1, quality:Quality.Dom13}, {root:6, quality:Quality.Maj7}, {root:11, quality:Quality.Major}],
        chordCount: 8,
        treblePattern: Pattern.Block,
        trebleSteps: 16,
        trebleSkip: [3, 2, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Rise,
        trebleVelocity: 55,
        trebleOctaves: 2,
        bassPattern: Pattern.PairsUp,
        bassSteps: 4,
        bassContour: Contour.Dip,
        bassVelocity: 44,
        octaveShift: 1,
        feel: Feel.Loose
    },
{
        name: "Spider",
        character: "A rising line (quarter-note triplets) over a rising bass (eighth-note triplets), straight timing, with rests.",
        bpm: 136,
        home: 3,
        chords: [{root:3, quality:Quality.Dom9}, {root:1, quality:Quality.Min9}, {root:3, quality:Quality.Major}, {root:4, quality:Quality.Maj13}, {root:3, quality:Quality.Dom9}, {root:1, quality:Quality.Min13}, {root:3, quality:Quality.Dom7}, {root:4, quality:Quality.Maj7}],
        chordCount: 8,
        treblePattern: Pattern.Climb,
        trebleSteps: 6,
        trebleSkip: [2, 7, 0],
        trebleSkipCount: 2,
        trebleContour: Contour.Fall,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Climb,
        bassSteps: 12,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Starter",
        character: "A rising line (half notes) over a block bass (quarter-note triplets), straight timing, wide treble, with rests.",
        bpm: 129,
        home: 5,
        chords: [{root:5, quality:Quality.Add4}, {root:10, quality:Quality.Add9}, {root:0, quality:Quality.Sus2}, {root:0, quality:Quality.Dom7Sus4}, {root:5, quality:Quality.Maj13}, {root:10, quality:Quality.Major}, {root:0, quality:Quality.Add13}, {root:0, quality:Quality.Maj7}],
        chordCount: 8,
        treblePattern: Pattern.Climb,
        trebleSteps: 2,
        trebleSkip: [5, 2, 2],
        trebleSkipCount: 3,
        trebleContour: Contour.Peak,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.Block,
        bassSteps: 6,
        bassContour: Contour.Dip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Steady",
        character: "Block chords (quarter notes) over rising bass pairs (sixteenth notes), straight timing, with rests.",
        bpm: 84,
        home: 1,
        chords: [{root:1, quality:Quality.Maj9}, {root:1, quality:Quality.Major}, {root:5, quality:Quality.Min9}, {root:5, quality:Quality.Min9}, {root:1, quality:Quality.Maj7}, {root:1, quality:Quality.Add4}, {root:5, quality:Quality.Min9}, {root:5, quality:Quality.Min13}],
        chordCount: 8,
        treblePattern: Pattern.Block,
        trebleSteps: 4,
        trebleSkip: [4, 8, 8],
        trebleSkipCount: 3,
        trebleContour: Contour.Rise,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.PairsUp,
        bassSteps: 16,
        bassContour: Contour.DoubleDip,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Straight
    },
{
        name: "Stutter",
        character: "Block chords (quarter notes) over a block bass (half notes), human timing, with rests.",
        bpm: 119,
        home: 7,
        chords: [{root:7, quality:Quality.Maj7}, {root:0, quality:Quality.Maj7}, {root:4, quality:Quality.Dom7Sus4}, {root:0, quality:Quality.Min9}, {root:7, quality:Quality.Add4}, {root:0, quality:Quality.Add9}, {root:4, quality:Quality.Dom9Sus4}, {root:0, quality:Quality.Min11}],
        chordCount: 8,
        treblePattern: Pattern.Block,
        trebleSteps: 4,
        trebleSkip: [6, 2, 4],
        trebleSkipCount: 3,
        trebleContour: Contour.DoublePeak,
        trebleVelocity: 74,
        trebleOctaves: 1,
        bassPattern: Pattern.Block,
        bassSteps: 2,
        bassContour: Contour.Peak,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Human
    },
{
        name: "Suspense",
        character: "Falling pairs (quarter-note triplets) over falling bass pairs (quarter notes), loose timing, wide treble, with rests.",
        bpm: 85,
        home: 11,
        chords: [{root:11, quality:Quality.Dom7Sus4}, {root:11, quality:Quality.Dom9Sus4}, {root:4, quality:Quality.Min7}, {root:4, quality:Quality.Min11}, {root:11, quality:Quality.Dom9Sus4}, {root:11, quality:Quality.Dom7Sus4}, {root:4, quality:Quality.Minor}, {root:4, quality:Quality.Min11}],
        chordCount: 8,
        treblePattern: Pattern.PairsDown,
        trebleSteps: 6,
        trebleSkip: [7, 3, 4],
        trebleSkipCount: 3,
        trebleContour: Contour.DoublePeak,
        trebleVelocity: 74,
        trebleOctaves: 2,
        bassPattern: Pattern.PairsDown,
        bassSteps: 4,
        bassContour: Contour.DoublePeak,
        bassVelocity: 60,
        octaveShift: 0,
        feel: Feel.Loose
    }
    ];

    function u32(value) { return value >>> 0; }

    function clampInt(value, lo, hi) {
        value = value | 0;
        if (value < lo) return lo;
        if (value > hi) return hi;
        return value;
    }

    function keyName(key) { return PITCHES[clampInt(key, 0, 11)]; }

    function formatChord(chord) {
        return PITCHES[((chord.root % 12) + 12) % 12] + SUFFIX[chord.quality];
    }

    function formatProgression(chords) {
        return chords.map(formatChord).join(", ");
    }

    function sameIgnoreCase(text, literal) {
        return text.toLowerCase() === literal;
    }

    function parseQuality(suffix) {
        if (suffix === "") return Quality.Major;
        if (suffix === "M7" || sameIgnoreCase(suffix, "maj7") || sameIgnoreCase(suffix, "major7")) return Quality.Maj7;
        if (suffix === "M9" || sameIgnoreCase(suffix, "maj9") || sameIgnoreCase(suffix, "major9")) return Quality.Maj9;
        if (suffix === "M13" || sameIgnoreCase(suffix, "maj13") || sameIgnoreCase(suffix, "major13")) return Quality.Maj13;
        if (sameIgnoreCase(suffix, "msus2")) return Quality.Sus2;
        if (sameIgnoreCase(suffix, "9sus4")) return Quality.Dom9Sus4;
        if (sameIgnoreCase(suffix, "7sus4")) return Quality.Dom7Sus4;
        if (sameIgnoreCase(suffix, "m13") || sameIgnoreCase(suffix, "min13")) return Quality.Min13;
        if (sameIgnoreCase(suffix, "m11") || sameIgnoreCase(suffix, "min11")) return Quality.Min11;
        if (sameIgnoreCase(suffix, "m9") || sameIgnoreCase(suffix, "min9")) return Quality.Min9;
        if (sameIgnoreCase(suffix, "m7") || sameIgnoreCase(suffix, "min7")) return Quality.Min7;
        if (sameIgnoreCase(suffix, "add13")) return Quality.Add13;
        if (sameIgnoreCase(suffix, "add9")) return Quality.Add9;
        if (sameIgnoreCase(suffix, "add4")) return Quality.Add4;
        if (sameIgnoreCase(suffix, "add2")) return Quality.Add2;
        if (sameIgnoreCase(suffix, "sus2")) return Quality.Sus2;
        if (sameIgnoreCase(suffix, "sus4") || sameIgnoreCase(suffix, "sus")) return Quality.Sus4;
        if (sameIgnoreCase(suffix, "dim")) return Quality.Dim;
        if (sameIgnoreCase(suffix, "aug") || suffix === "+") return Quality.Aug;
        if (sameIgnoreCase(suffix, "13")) return Quality.Dom13;
        if (sameIgnoreCase(suffix, "11")) return Quality.Dom11;
        if (sameIgnoreCase(suffix, "9")) return Quality.Dom9;
        if (sameIgnoreCase(suffix, "7")) return Quality.Dom7;
        if (sameIgnoreCase(suffix, "m") || sameIgnoreCase(suffix, "min") || suffix === "-") return Quality.Minor;
        return null;
    }

    function letterPc(letter) {
        switch (letter.toUpperCase()) {
            case "C": return 0;
            case "D": return 2;
            case "E": return 4;
            case "F": return 5;
            case "G": return 7;
            case "A": return 9;
            case "B": return 11;
            default: return -1;
        }
    }

    function parseToken(token) {
        if (!token) return null;
        const rootPc = letterPc(token[0]);
        if (rootPc < 0) return null;
        let index = 1;
        let pc = rootPc;
        if (index < token.length && (token[index] === "#" || token[index] === "b")) {
            pc = token[index] === "#" ? (pc + 1) % 12 : (pc + 11) % 12;
            index += 1;
        }
        let suffix = token.slice(index);
        const slash = suffix.indexOf("/");
        if (slash >= 0) suffix = suffix.slice(0, slash);
        suffix = suffix.trim();
        const quality = parseQuality(suffix);
        if (quality === null) return null;
        return { root: pc, quality: quality };
    }

    function parseProgression(text) {
        const out = [];
        let token = "";
        let ok = true;
        function flush() {
            const trimmed = token.trim();
            token = "";
            if (!trimmed) return;
            const chord = parseToken(trimmed);
            if (!chord) ok = false;
            else out.push(chord);
        }
        for (let i = 0; i < text.length; i += 1) {
            const ch = text[i];
            if (ch === "," || ch === "\n") flush();
            else token += ch;
        }
        flush();
        if (!ok || out.length === 0) return null;
        return out;
    }

    function styleAt(index) {
        if (index < 0) index = 0;
        if (index >= STYLES.length) index = STYLES.length - 1;
        return STYLES[index];
    }

    function styleProgression(style, keyIndex) {
        const delta = (clampInt(keyIndex, 0, 11) - style.home + 12) % 12;
        const count = Math.max(0, Math.min(8, style.chordCount));
        const chords = [];
        for (let i = 0; i < count; i += 1) {
            chords.push({ root: (style.chords[i].root + delta) % 12, quality: style.chords[i].quality });
        }
        return chords;
    }

    function Rng(seed) {
        this.state = seed === 0 ? 1 : u32(seed);
    }
    Rng.prototype.next = function () {
        let state = this.state;
        state = u32(state ^ u32(state << 13));
        state = u32(state ^ (state >>> 17));
        state = u32(state ^ u32(state << 5));
        this.state = state;
        return state;
    };
    Rng.prototype.below = function (count) { return this.next() % count; };

    function generateProgression(keyIndex, seed) {
        const rng = new Rng(seed);
        const bag = [0, 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5];
        const degrees = [];
        let previous = -1;
        for (let i = 0; i < 4; i += 1) {
            let degree = bag[rng.below(12)];
            for (let guard = 0; guard < 8 && degree === previous; guard += 1)
                degree = bag[rng.below(12)];
            previous = degree;
            degrees.push(degree);
        }
        const key = clampInt(keyIndex, 0, 11);
        const chords = [];
        const firstPass = [];
        for (let i = 0; i < 4; i += 1) {
            const quality = QUALITY_BAG[degrees[i]][rng.below(QUALITY_BAG[degrees[i]].length)];
            firstPass.push(quality);
            chords.push({ root: (key + SCALE[degrees[i]]) % 12, quality: quality });
        }
        for (let i = 0; i < 4; i += 1) {
            let quality = QUALITY_BAG[degrees[i]][rng.below(QUALITY_BAG[degrees[i]].length)];
            if (quality === firstPass[i])
                quality = QUALITY_BAG[degrees[i]][rng.below(QUALITY_BAG[degrees[i]].length)];
            chords.push({ root: (key + SCALE[degrees[i]]) % 12, quality: quality });
        }
        return chords;
    }

    function foldPitch(note) {
        while (note > 96) note -= 12;
        while (note < 36) note += 12;
        return note;
    }

    function uniqueSorted(notes) {
        notes.sort(function (a, b) { return a - b; });
        const out = [];
        for (let i = 0; i < notes.length; i += 1) {
            if (i === 0 || notes[i] !== notes[i - 1]) out.push(notes[i]);
        }
        return out;
    }

    function shellNotes(intervals, rootMidi) {
        const notes = [rootMidi];
        let third = -1;
        let seventh = -1;
        let fifth = -1;
        let sus = -1;
        for (let i = 0; i < intervals.length; i += 1) {
            const interval = intervals[i];
            const pc = ((interval % 12) + 12) % 12;
            const note = rootMidi + interval;
            if ((pc === 3 || pc === 4) && third < 0) third = note;
            else if ((pc === 10 || pc === 11) && seventh < 0) seventh = note;
            else if (pc === 7 && fifth < 0) fifth = note;
            else if ((pc === 2 || pc === 5) && sus < 0) sus = note;
        }
        if (third >= 0) notes.push(third);
        else if (sus >= 0) notes.push(sus);
        if (seventh >= 0) notes.push(seventh);
        else if (notes.length < 3 && fifth >= 0) notes.push(fifth);
        return uniqueSorted(notes);
    }

    function voiceChord(chord, density, octaveShift, trebleOctaves) {
        const intervals = INTERVALS[chord.quality];
        const rootMidi = 60 + (chord.root % 12);
        let treble = [];
        let bass = [];
        if (density === Density.Shell) {
            treble = shellNotes(intervals, rootMidi);
        } else {
            for (let i = 0; i < intervals.length; i += 1) treble.push(rootMidi + intervals[i]);
            treble = uniqueSorted(treble);
            if (density === Density.Full && trebleOctaves >= 2) {
                const upper = treble.slice();
                for (let i = 0; i < upper.length; i += 1) {
                    if (upper[i] + 12 <= 96) treble.push(upper[i] + 12);
                }
                treble = uniqueSorted(treble);
            }
        }
        if (density === Density.Full) {
            const bassRoot = rootMidi - 12;
            bass.push(bassRoot);
            for (let i = 0; i < intervals.length; i += 1) {
                if ((((intervals[i] % 12) + 12) % 12) === 7) {
                    bass.push(bassRoot + 7);
                    break;
                }
            }
        }
        const shift = octaveShift * 12;
        for (let i = 0; i < treble.length; i += 1) treble[i] = foldPitch(treble[i] + shift);
        for (let i = 0; i < bass.length; i += 1) bass[i] = foldPitch(bass[i] + shift);
        return { treble: uniqueSorted(treble), bass: uniqueSorted(bass) };
    }

    function pickNotes(pattern, ordered, cycle) {
        if (ordered.length === 0) return [];
        const count = ordered.length;
        if (pattern === Pattern.Block) return ordered.slice();
        if (pattern === Pattern.Climb) return [ordered[cycle % count]];
        if (pattern === Pattern.Fall) return [ordered[count - 1 - (cycle % count)]];
        if (pattern === Pattern.PairsUp) {
            if (count < 2) return [ordered[0]];
            const start = cycle % (count - 1);
            return [ordered[start], ordered[start + 1]];
        }
        if (count < 2) return [ordered[count - 1]];
        const start = cycle % (count - 1);
        return [ordered[count - 1 - start], ordered[count - 2 - start]];
    }

    function mix32(value) {
        value = u32(value);
        value = u32(value ^ (value >>> 16));
        value = u32(Math.imul(value, 0x7feb352d));
        value = u32(value ^ (value >>> 15));
        value = u32(Math.imul(value, 0x846ca68b));
        value = u32(value ^ (value >>> 16));
        return value;
    }

    function laidBack(feel, chordIndex, step, stepBeats, salt) {
        if (feel === Feel.Straight || stepBeats <= 0) return 0;
        const hash = mix32(u32((u32(chordIndex) * 131) ^ (u32(step) * 17) ^ salt));
        if ((hash % 100) >= 58) return 0;
        const amount = feel === Feel.Loose ? 0.13 : 0.055;
        const fraction = ((hash >>> 8) % 1000) / 1000;
        return stepBeats * amount * fraction;
    }

    function f(value) { return Math.fround(value); }

    function contourGain(contour, position) {
        position = f(Math.min(1, Math.max(0, position)));
        const blend = function (from, to, t) { return f(f(from) + f(f(to) - f(from)) * f(t)); };
        const hump = function (t) {
            t = f(t);
            return t < 0.5 ? f(t * 2) : f(f(1 - t) * 2);
        };
        if (contour === Contour.Rise) return blend(0.62, 1.08, position);
        if (contour === Contour.Fall) return blend(1.08, 0.62, position);
        if (contour === Contour.Peak) return blend(0.62, 1.08, hump(position));
        if (contour === Contour.Dip) return blend(1.08, 0.62, hump(position));
        if (contour === Contour.DoublePeak) return blend(0.66, 1.06, hump(f(f(position * 2) % 1)));
        if (contour === Contour.DoubleDip) return blend(1.06, 0.66, hump(f(f(position * 2) % 1)));
        return f(1);
    }

    function clamp01(value) { return Math.min(1, Math.max(0, value)); }
    function dbToLinear(db) { return Math.pow(10, db / 20); }
    function shapedRise(pos, minDb, span) { return dbToLinear(minDb + Math.pow(clamp01(pos), 2) * span); }
    function shapedFall(pos, maxDb, span) { return dbToLinear(maxDb - Math.pow(clamp01(pos), 2) * span); }
    function valleyGain(pos, minDb, maxDb, span) {
        return pos <= 0.5 ? shapedFall(pos * 2, maxDb, span) : shapedRise((pos - 0.5) * 2, minDb, span);
    }
    function hillGain(pos, minDb, maxDb, span) {
        return pos <= 0.5 ? shapedRise(pos * 2, minDb, span) : shapedFall((pos - 0.5) * 2, maxDb, span);
    }

    function everyBarGain(shape, position, intensity) {
        if (shape === EveryBar.Style || shape === EveryBar.None || shape == null) return 1;
        const pos = clamp01(position);
        const minDb = 20 * Math.log10(0.05);
        const maxDb = 20 * Math.log10(1.3);
        const span = maxDb - minDb;
        let raw = 1;
        if (shape === EveryBar.Uphill) raw = shapedRise(pos, minDb, span);
        else if (shape === EveryBar.Downhill) raw = shapedFall(pos, maxDb, span);
        else if (shape === EveryBar.Valley) raw = valleyGain(pos, minDb, maxDb, span);
        else if (shape === EveryBar.Hill) raw = hillGain(pos, minDb, maxDb, span);
        else if (shape === EveryBar.TwoValley) raw = valleyGain((pos * 2) % 1, minDb, maxDb, span);
        else if (shape === EveryBar.TwoHill) {
            const phase = (pos * 4) % 2;
            raw = phase <= 1 ? shapedRise(phase, minDb, span) : shapedFall(phase - 1, maxDb, span);
        }
        if (intensity <= 0) return 0;
        if (intensity >= 1) return raw;
        return intensity * (raw - 1) + 1;
    }

    function emitPart(notes, pool, pattern, steps, skip, skipCount, contour, everyBar, everyBarIntensity, baseVelocity, chordIndex, phraseBeats, feel, salt) {
        if (pool.length === 0 || steps < 1) return;
        const stepBeats = 4 / steps;
        let counter = 0;
        let skipIndex = 0;
        for (let step = 0; step < steps; step += 1) {
            let silent = false;
            if (skipCount > 0 && skip) {
                counter += 1;
                let target = skip[skipIndex];
                if (target < 1) target = 1;
                if (counter >= target) {
                    counter = 0;
                    skipIndex = (skipIndex + 1) % skipCount;
                    silent = true;
                }
            }
            if (silent) continue;
            const at = step * stepBeats;
            const chosen = pickNotes(pattern, pool, step);
            const when = chordIndex * 4 + at + laidBack(feel, chordIndex, step, stepBeats, salt);
            const length = Math.max(0.04, stepBeats * 0.84);
            let start = when;
            if (start < 0) start = 0;
            if (phraseBeats > 0 && start >= phraseBeats) start = Math.max(0, phraseBeats - length);
            const barPos = at / 4;
            let velocity;
            if (everyBar == null || everyBar === EveryBar.Style) {
                velocity = Math.round(baseVelocity * contourGain(contour, f(barPos)));
                if (velocity < 1) velocity = 1;
                if (velocity > 127) velocity = 127;
            } else {
                velocity = Math.round(baseVelocity * everyBarGain(everyBar, barPos, everyBarIntensity));
                if (velocity < 0) velocity = 0;
                if (velocity > 127) velocity = 127;
            }
            for (let i = 0; i < chosen.length; i += 1) {
                notes.push({ startBeats: start, lengthBeats: length, midi: chosen[i], velocity: velocity });
            }
        }
    }

    function resolveOverlaps(notes) {
        notes.sort(function (a, b) {
            if (a.startBeats !== b.startBeats) return a.startBeats < b.startBeats ? -1 : 1;
            return a.midi - b.midi;
        });
        const resolved = [];
        for (let n = 0; n < notes.length; n += 1) {
            const event = notes[n];
            let keepLooking = true;
            while (keepLooking) {
                keepLooking = false;
                for (let i = resolved.length - 1; i >= 0; i -= 1) {
                    if (resolved[i].midi !== event.midi) continue;
                    const previous = resolved[i];
                    if (previous.startBeats + previous.lengthBeats > event.startBeats + 1.0e-4) {
                        const shortened = event.startBeats - previous.startBeats;
                        if (shortened < 0.02) resolved.splice(i, 1);
                        else previous.lengthBeats = shortened;
                        keepLooking = true;
                    }
                    break;
                }
            }
            resolved.push({
                startBeats: event.startBeats,
                lengthBeats: event.lengthBeats,
                gateBeats: event.lengthBeats,
                midi: event.midi,
                velocity: event.velocity
            });
        }
        return resolved;
    }

    function envelopeAt(envelope, gateBeats, beatsFromStart) {
        if (beatsFromStart < 0 || gateBeats < 0) return 0;
        const attack = Math.max(0, envelope.attack || 0);
        const decay = Math.max(0, envelope.decay || 0);
        const sustain = Math.min(1, Math.max(0, envelope.sustain == null ? 1 : envelope.sustain));
        const release = Math.max(0, envelope.release || 0);
        if (beatsFromStart > gateBeats + release) return 0;
        function shape(time) {
            if (time < 0) return 0;
            if (attack <= 0 && decay <= 0) return sustain;
            if (attack > 0 && time < attack) return time / attack;
            if (decay > 0 && time < attack + decay) {
                const u = (time - attack) / decay;
                return 1 + (sustain - 1) * u;
            }
            return sustain;
        }
        if (beatsFromStart < gateBeats || release <= 0) {
            if (release <= 0 && beatsFromStart >= gateBeats) return 0;
            return Math.min(1, Math.max(0, shape(beatsFromStart)));
        }
        const atRelease = Math.min(1, Math.max(0, shape(gateBeats)));
        const u = (beatsFromStart - gateBeats) / release;
        if (u >= 1) return 0;
        return Math.min(1, Math.max(0, atRelease * (1 - u)));
    }

    function pushRamp(times, from, to) {
        if (to < from) return;
        if (to <= from + 1e-9) {
            times.push(from);
            return;
        }
        const steps = 8;
        for (let i = 0; i <= steps; i += 1)
            times.push(from + (to - from) * (i / steps));
    }

    function applyDynamics(phrase, requested, requestedVolume) {
        const envelope = {
            attack: Math.max(0, requested && requested.attack ? requested.attack : 0),
            decay: Math.max(0, requested && requested.decay ? requested.decay : 0),
            sustain: Math.min(1, Math.max(0, !requested || requested.sustain == null ? 1 : requested.sustain)),
            release: Math.max(0, requested && requested.release ? requested.release : 0)
        };
        const volume = clampInt(Math.round(requestedVolume == null ? 100 : requestedVolume), 0, 100);
        const gain = volume / 100;
        phrase.envelope = envelope;
        phrase.volume = volume;
        phrase.notes.forEach(function (note) { note.gateBeats = note.lengthBeats; });
        phrase.notes.forEach(function (note, index) {
            let room = 1e9;
            phrase.notes.forEach(function (other, otherIndex) {
                if (otherIndex === index || other.midi !== note.midi) return;
                const gap = other.startBeats - (note.startBeats + note.gateBeats);
                if (gap >= -1e-4 && gap < room) room = gap;
            });
            let tail = envelope.release;
            if (room < 1e8) tail = Math.min(tail, Math.max(0, room - 1e-4));
            if (tail < 1e-4) tail = 0;
            note.lengthBeats = note.gateBeats + tail;
        });
        phrase.notes = phrase.notes.filter(function (note) {
            let velocity = Math.round(note.velocity * gain);
            if (velocity < 0) velocity = 0;
            if (velocity > 127) velocity = 127;
            note.velocity = velocity;
            return velocity > 0;
        });
        let soundingEnd = phrase.lengthBeats;
        phrase.notes.forEach(function (note) {
            soundingEnd = Math.max(soundingEnd, note.startBeats + note.lengthBeats);
        });
        phrase.lengthBeats = soundingEnd;
        const times = [];
        phrase.notes.forEach(function (note) {
            const gate = note.gateBeats;
            const tail = Math.max(0, note.lengthBeats - gate);
            times.push(note.startBeats);
            if (envelope.attack > 0) pushRamp(times, note.startBeats, note.startBeats + Math.min(envelope.attack, gate));
            const decayFrom = note.startBeats + Math.min(envelope.attack, gate);
            const decayTo = note.startBeats + Math.min(envelope.attack + envelope.decay, gate);
            if (envelope.decay > 0 && decayTo > decayFrom) pushRamp(times, decayFrom, decayTo);
            times.push(note.startBeats + gate);
            if (tail > 0) pushRamp(times, note.startBeats + gate, note.startBeats + gate + tail);
        });
        times.sort(function (a, b) { return a - b; });
        phrase.expression = [];
        let previous = -1;
        let previousTime = -1;
        times.forEach(function (time) {
            if (previousTime >= 0 && Math.abs(time - previousTime) < 1e-6) return;
            previousTime = time;
            let level = 0;
            phrase.notes.forEach(function (note) {
                const local = time - note.startBeats;
                if (local < -1e-8 || local > note.lengthBeats + 1e-8) return;
                level = Math.max(level, envelopeAt(envelope, note.gateBeats, local));
            });
            let value = Math.round(127 * Math.min(1, Math.max(0, level)) * gain);
            if (value < 0) value = 0;
            if (value > 127) value = 127;
            if (value === previous) return;
            phrase.expression.push({ beats: time, value: value });
            previous = value;
        });
    }

    function renderPhrase(style, chords, bpm, feel, density, rhythm, envelope, volume, hills) {
        const phrase = {
            notes: [],
            expression: [],
            lengthBeats: chords.length * 4,
            bpm: clampInt(Math.round(bpm), 40, 200),
            chordCount: chords.length,
            volume: 100,
            envelope: { attack: 0, decay: 0, sustain: 1, release: 0 }
        };
        if (chords.length === 0) return phrase;
        let treblePattern = style.treblePattern;
        let trebleSteps = Math.max(1, style.trebleSteps);
        const trebleSkip = style.trebleSkip.slice();
        let trebleSkipCount = style.trebleSkipCount;
        let bassPattern = style.bassPattern;
        let bassSteps = Math.max(1, style.bassSteps);
        if (rhythm === Rhythm.Block) {
            treblePattern = Pattern.Block;
            bassPattern = Pattern.Block;
            trebleSteps = 4;
            bassSteps = 4;
            trebleSkipCount = 0;
        } else if (rhythm === Rhythm.Climb) {
            treblePattern = Pattern.Climb;
            bassPattern = Pattern.Climb;
            trebleSteps = 8;
            bassSteps = 4;
            trebleSkipCount = 0;
        } else if (rhythm === Rhythm.Fall) {
            treblePattern = Pattern.Fall;
            bassPattern = Pattern.Fall;
            trebleSteps = 8;
            bassSteps = 4;
            trebleSkipCount = 0;
        } else if (rhythm === Rhythm.Pairs) {
            treblePattern = Pattern.PairsUp;
            bassPattern = Pattern.PairsDown;
            trebleSteps = 8;
            bassSteps = 4;
            trebleSkipCount = 0;
        }
        const octaves = density === Density.Full ? style.trebleOctaves : 1;
        const trebleEvery = hills && hills.treble != null ? hills.treble : EveryBar.Style;
        const bassEvery = hills && hills.bass != null ? hills.bass : EveryBar.Style;
        const trebleIntensity = hills && hills.trebleIntensity != null ? hills.trebleIntensity : 0.33;
        const bassIntensity = hills && hills.bassIntensity != null ? hills.bassIntensity : 0.33;
        for (let chordIndex = 0; chordIndex < phrase.chordCount; chordIndex += 1) {
            const pools = voiceChord(chords[chordIndex], density, style.octaveShift, octaves);
            emitPart(phrase.notes, pools.treble, treblePattern, trebleSteps, trebleSkip, trebleSkipCount,
                style.trebleContour, trebleEvery, trebleIntensity, style.trebleVelocity, chordIndex, phrase.lengthBeats, feel, 0xA11);
            emitPart(phrase.notes, pools.bass, bassPattern, bassSteps, null, 0,
                style.bassContour, bassEvery, bassIntensity, style.bassVelocity, chordIndex, phrase.lengthBeats, feel, 0xB22);
        }
        phrase.notes = resolveOverlaps(phrase.notes);
        applyDynamics(phrase, envelope, volume);
        return phrase;
    }

    function push32(out, value) {
        out.push((value >>> 24) & 0xff, (value >>> 16) & 0xff, (value >>> 8) & 0xff, value & 0xff);
    }
    function push16(out, value) {
        out.push((value >> 8) & 0xff, value & 0xff);
    }
    function pushVarLen(out, value) {
        value = u32(value);
        let buffer = value & 0x7f;
        while ((value = u32(value >>> 7)) !== 0) {
            buffer = (buffer << 8) | ((value & 0x7f) | 0x80);
            buffer = buffer >>> 0;
        }
        while (true) {
            out.push(buffer & 0xff);
            if (buffer & 0x80) buffer = buffer >>> 8;
            else break;
        }
    }

    function renderSmf(phrase) {
        const ticksPerQuarter = 960;
        const bpm = clampInt(phrase.bpm, 1, 999);
        const lengthBeats = phrase.lengthBeats > 0 ? phrase.lengthBeats : 4;
        let endTick = Math.round(lengthBeats * ticksPerQuarter);
        const events = [];
        for (let i = 0; i < phrase.notes.length; i += 1) {
            const note = phrase.notes[i];
            let midi = note.midi;
            if (midi < 0) midi = 0;
            if (midi > 127) midi = 127;
            let velocity = note.velocity;
            if (velocity < 0) velocity = 0;
            if (velocity > 127) velocity = 127;
            if (velocity <= 0) continue;
            const onTick = Math.round(Math.max(0, note.startBeats) * ticksPerQuarter);
            let offTick = Math.round(Math.max(0, note.startBeats + note.lengthBeats) * ticksPerQuarter);
            if (offTick <= onTick) offTick = onTick + 1;
            events.push({ tick: onTick, order: 2, status: 0x90, data1: midi, data2: velocity, index: i * 2 });
            events.push({ tick: offTick, order: 0, status: 0x80, data1: midi, data2: 0, index: i * 2 + 1 });
            if (offTick > endTick) endTick = offTick;
        }
        (phrase.expression || []).forEach(function (expression, index) {
            let value = expression.value;
            if (value < 0) value = 0;
            if (value > 127) value = 127;
            const tick = Math.round(Math.max(0, expression.beats) * ticksPerQuarter);
            events.push({ tick: tick, order: 1, status: 0xb0, data1: 11, data2: value, index: 100000 + index });
            if (tick > endTick) endTick = tick;
        });
        events.sort(function (a, b) {
            if (a.tick !== b.tick) return a.tick - b.tick;
            if (a.order !== b.order) return a.order - b.order;
            return a.index - b.index;
        });
        const track = [];
        const microseconds = Math.round(60000000 / bpm);
        pushVarLen(track, 0);
        track.push(0xff, 0x51, 0x03, (microseconds >> 16) & 0xff, (microseconds >> 8) & 0xff, microseconds & 0xff);
        pushVarLen(track, 0);
        track.push(0xff, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08);
        pushVarLen(track, 0);
        track.push(0xff, 0x03, 10);
        const name = "Chord2MIDI";
        for (let i = 0; i < name.length; i += 1) track.push(name.charCodeAt(i));
        let lastTick = 0;
        for (let i = 0; i < events.length; i += 1) {
            pushVarLen(track, events[i].tick - lastTick);
            lastTick = events[i].tick;
            track.push(events[i].status, events[i].data1, events[i].data2);
        }
        pushVarLen(track, endTick >= lastTick ? endTick - lastTick : 0);
        track.push(0xff, 0x2f, 0x00);
        const file = [0x4d, 0x54, 0x68, 0x64];
        push32(file, 6);
        push16(file, 1);
        push16(file, 1);
        push16(file, ticksPerQuarter);
        file.push(0x4d, 0x54, 0x72, 0x6b);
        push32(file, track.length);
        return file.concat(track);
    }

    return {
        Quality: Quality,
        Pattern: Pattern,
        Contour: Contour,
        EveryBar: EveryBar,
        Feel: Feel,
        Density: Density,
        Rhythm: Rhythm,
        styles: STYLES,
        keyName: keyName,
        formatChord: formatChord,
        formatProgression: formatProgression,
        parseProgression: parseProgression,
        styleAt: styleAt,
        styleProgression: styleProgression,
        generateProgression: generateProgression,
        renderPhrase: renderPhrase,
        renderSmf: renderSmf,
        envelopeAt: envelopeAt,
        everyBarGain: everyBarGain
    };
});
