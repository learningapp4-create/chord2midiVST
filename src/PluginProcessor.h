#pragma once

#include "Types.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <array>
#include <atomic>
#include <memory>
#include <mutex>

class PluginEditor;

class PluginProcessor : public juce::AudioProcessor,
                        private juce::AudioProcessorValueTreeState::Listener,
                        private juce::AsyncUpdater {
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Chord2MIDI"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    struct Model {
        int textRevision = 0;
        std::string chords;
        std::string status;
        std::string character;
    };

    Model model() const;
    void setCustomChords(const std::string& text);
    void useStyleLine();
    void generateNewProgression();
    void setAudition(bool shouldPlay);
    bool isAudition() const { return audition.load(); }
    std::vector<unsigned char> currentMidiFile() const;

private:
    struct Voice {
        bool on = false;
        int note = 60;
        int samplesLeft = 0;
        int startSample = 0;
        int age = 0;
        float phase = 0.0f;
        float amp = 0.0f;
    };

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void handleAsyncUpdate() override;

    int readChoice(const char* parameterID, int count) const;
    void rebuildNow();
    void applyStyle(int styleIndex, int keyIndex);
    void renderAndPublish();
    void publishProperties();
    void releaseAll(juce::MidiBuffer& midi, int sample);
    void noteOn(juce::MidiBuffer& midi, int sample, int note, int velocity, int durationSamples);
    void playFrom(const c2m::Phrase& phrase, double startBeat, double beatSpan, double bpm, juce::MidiBuffer& midi);
    void renderVoices(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::vector<c2m::Chord> progression;
    std::string storedText;
    std::string statusText;
    std::string characterText;
    std::string errorText;
    int textRevision = 0;
    int appliedStyle = -1;
    int appliedKey = -1;
    bool chordsAreCustom = false;
    std::uint32_t seed = 1;
    bool updatingParams = false;
    bool restoring = false;

    mutable std::mutex modelLock;
    mutable juce::SpinLock phraseLock;
    std::shared_ptr<const c2m::Phrase> phrase;
    int phraseToken = 0;

    double sampleRate = 44100.0;
    int currentBlockSamples = 0;
    double cursor = 0.0;
    bool seenHost = false;
    double expectedHost = 0.0;
    int armedToken = -1;
    std::atomic<bool> audition { false };
    std::array<Voice, 32> voices {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
