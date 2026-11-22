#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace {
juce::StringArray choiceNames(std::initializer_list<const char*> names) {
    juce::StringArray out;
    for (const char* name : names)
        out.add(name);
    return out;
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createLayout() {
    juce::StringArray styles;
    for (int i = 0; i < c2m::kStyleCount; ++i)
        styles.add(c2m::styleAt(i).name);
    juce::StringArray keys;
    for (int i = 0; i < 12; ++i)
        keys.add(c2m::keyName(i));

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"style", 1}, "Style", styles, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"key", 1}, "Key", keys, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"tempo", 1}, "Tempo", juce::NormalisableRange<float>(40.0f, 200.0f, 1.0f), 71.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"feel", 1}, "Feel", choiceNames({"Straight", "Human", "Loose"}), 1));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"density", 1}, "Voicing", choiceNames({"Shell", "Close", "Full"}), 2));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"rhythm", 1}, "Rhythm", choiceNames({"Style", "Block", "Climb", "Fall", "Pairs"}), 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"volume", 1}, "Phrase volume", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"attack", 1}, "Attack", juce::NormalisableRange<float>(0.0f, 4.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"decay", 1}, "Decay", juce::NormalisableRange<float>(0.0f, 4.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"sustain", 1}, "Sustain", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"release", 1}, "Release", juce::NormalisableRange<float>(0.0f, 4.0f, 0.01f), 0.0f));
    const auto everyBar = choiceNames({"Style", "none", "uphill", "downhill", "valley", "hill", "2valley", "2hill"});
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"trebleEveryBar", 1}, "Treble every-bar", everyBar, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"bassEveryBar", 1}, "Bass every-bar", everyBar, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"trebleFilterIntensity", 1}, "Treble filter intensity",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 33.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bassFilterIntensity", 1}, "Bass filter intensity",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 33.0f));
    return {params.begin(), params.end()};
}

PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createLayout()),
      phrase(std::make_shared<const c2m::Phrase>()) {
    for (const char* id : {"style", "key", "tempo", "feel", "density", "rhythm", "volume", "attack", "decay", "sustain", "release",
                           "trebleEveryBar", "bassEveryBar", "trebleFilterIntensity", "bassFilterIntensity"})
        apvts.addParameterListener(id, this);
    rebuildNow();
}

PluginProcessor::~PluginProcessor() {
    cancelPendingUpdate();
    for (const char* id : {"style", "key", "tempo", "feel", "density", "rhythm", "volume", "attack", "decay", "sustain", "release",
                           "trebleEveryBar", "bassEveryBar", "trebleFilterIntensity", "bassFilterIntensity"})
        apvts.removeParameterListener(id, this);
}

void PluginProcessor::prepareToPlay(double rate, int) {
    sampleRate = rate > 0.0 ? rate : 44100.0;
    for (auto& voice : voices)
        voice.on = false;
}

void PluginProcessor::releaseResources() {
    for (auto& voice : voices)
        voice.on = false;
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto& output = layouts.getMainOutputChannelSet();
    const bool outputOk = output.isDisabled()
                          || output == juce::AudioChannelSet::mono()
                          || output == juce::AudioChannelSet::stereo();
    return outputOk && layouts.getMainInputChannelSet().isDisabled();
}

int PluginProcessor::readChoice(const char* parameterID, int count) const {
    const auto* value = apvts.getRawParameterValue(parameterID);
    int index = value == nullptr ? 0 : static_cast<int>(std::lround(value->load()));
    if (index < 0) index = 0;
    if (count > 0 && index >= count) index = count - 1;
    return index;
}

void PluginProcessor::parameterChanged(const juce::String&, float) {
    if (updatingParams || restoring) return;
    triggerAsyncUpdate();
}

void PluginProcessor::handleAsyncUpdate() {
    rebuildNow();
}

void PluginProcessor::applyStyle(int styleIndex, int keyIndex) {
    const auto& style = c2m::styleAt(styleIndex);
    updatingParams = true;
    if (auto* tempo = apvts.getParameter("tempo"))
        tempo->setValueNotifyingHost(tempo->convertTo0to1(static_cast<float>(style.defaultBpm)));
    if (auto* feel = apvts.getParameter("feel"))
        feel->setValueNotifyingHost(feel->convertTo0to1(static_cast<float>(static_cast<int>(style.defaultFeel))));
    updatingParams = false;

    progression = c2m::styleProgression(style, keyIndex);
    chordsAreCustom = false;
    storedText = c2m::formatProgression(progression);
    errorText.clear();
    ++textRevision;
    appliedStyle = styleIndex;
    appliedKey = keyIndex;
}

void PluginProcessor::rebuildNow() {
    if (updatingParams || restoring) return;
    const int style = readChoice("style", c2m::kStyleCount);
    const int key = readChoice("key", 12);
    if (style != appliedStyle) {
        applyStyle(style, key);
    } else if (key != appliedKey && appliedKey >= 0) {
        const int delta = (key - appliedKey + 12) % 12;
        for (auto& chord : progression)
            chord.root = (chord.root + delta) % 12;
        appliedKey = key;
        storedText = c2m::formatProgression(progression);
        errorText.clear();
        ++textRevision;
    } else if (appliedKey < 0) {
        appliedKey = key;
    }
    renderAndPublish();
}

void PluginProcessor::renderAndPublish() {
    const int styleIndex = appliedStyle < 0 ? 0 : appliedStyle;
    const int keyIndex = appliedKey < 0 ? 0 : appliedKey;
    const auto& style = c2m::styleAt(styleIndex);
    const int bpm = std::clamp(static_cast<int>(std::lround(apvts.getRawParameterValue("tempo")->load())), 40, 200);
    const auto feel = static_cast<c2m::Feel>(readChoice("feel", 3));
    const auto density = static_cast<c2m::Density>(readChoice("density", 3));
    const auto rhythm = static_cast<c2m::RhythmChoice>(readChoice("rhythm", 5));
    const auto readFloat = [this](const char* parameterID, float fallback) {
        const auto* value = apvts.getRawParameterValue(parameterID);
        return value == nullptr ? fallback : value->load();
    };
    c2m::Envelope envelope;
    envelope.attack = std::clamp(static_cast<double>(readFloat("attack", 0.0f)), 0.0, 4.0);
    envelope.decay = std::clamp(static_cast<double>(readFloat("decay", 0.0f)), 0.0, 4.0);
    envelope.sustain = std::clamp(static_cast<double>(readFloat("sustain", 100.0f)), 0.0, 100.0) / 100.0;
    envelope.release = std::clamp(static_cast<double>(readFloat("release", 0.0f)), 0.0, 4.0);
    const int volume = std::clamp(static_cast<int>(std::lround(readFloat("volume", 100.0f))), 0, 100);
    c2m::HillSettings hills;
    hills.treble = static_cast<c2m::EveryBar>(readChoice("trebleEveryBar", 8));
    hills.bass = static_cast<c2m::EveryBar>(readChoice("bassEveryBar", 8));
    hills.trebleIntensity = std::clamp(static_cast<double>(readFloat("trebleFilterIntensity", 33.0f)), 0.0, 100.0) / 100.0;
    hills.bassIntensity = std::clamp(static_cast<double>(readFloat("bassFilterIntensity", 33.0f)), 0.0, 100.0) / 100.0;
    auto rendered = std::make_shared<c2m::Phrase>(
        c2m::renderPhrase(style, progression, bpm, feel, density, rhythm, envelope, volume, hills));

    std::string status;
    if (!errorText.empty()) {
        status = errorText;
    } else {
        status = style.name;
        if (keyIndex == style.homeRoot)
            status += " in " + std::string(c2m::keyName(keyIndex));
        else
            status += " in " + std::string(c2m::keyName(keyIndex)) + " (catalogued in " + c2m::keyName(style.homeRoot) + ")";
        status += " · " + std::string(c2m::toString(feel)) + " · " + c2m::toString(density) + " voicing · "
                  + c2m::toString(rhythm) + " rhythm · " + std::to_string(bpm) + " BPM · "
                  + std::to_string(volume) + "% volume · "
                  + std::to_string(rendered->notes.size()) + " notes · one chord per bar";
        if (hills.treble != c2m::EveryBar::Style || hills.bass != c2m::EveryBar::Style) {
            status += " · treble every-bar " + std::string(c2m::toString(hills.treble))
                      + " · bass every-bar " + std::string(c2m::toString(hills.bass));
        }
    }

    std::string character = style.character == nullptr ? std::string() : style.character;
    if (rhythm != c2m::RhythmChoice::Style)
        character = "Rhythm override is on, so this style's pattern is replaced. " + character;

    {
        const std::lock_guard<std::mutex> guard(modelLock);
        statusText = std::move(status);
        characterText = std::move(character);
    }
    {
        const juce::SpinLock::ScopedLockType guard(phraseLock);
        phrase = std::move(rendered);
        ++phraseToken;
    }
    publishProperties();
}

void PluginProcessor::publishProperties() {
    apvts.state.setProperty("chords", juce::String(storedText), nullptr);
    apvts.state.setProperty("custom", chordsAreCustom, nullptr);
    apvts.state.setProperty("seed", static_cast<int>(seed), nullptr);
}

PluginProcessor::Model PluginProcessor::model() const {
    const std::lock_guard<std::mutex> guard(modelLock);
    Model snapshot;
    snapshot.textRevision = textRevision;
    snapshot.chords = storedText;
    snapshot.status = statusText;
    snapshot.character = characterText;
    return snapshot;
}

void PluginProcessor::setCustomChords(const std::string& text) {
    std::string trimmed = text;
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())))
        trimmed.pop_back();
    if (!trimmed.empty() && trimmed.back() == ',')
        return;

    std::vector<c2m::Chord> parsed;
    if (!c2m::parseProgression(text, parsed)) {
        errorText = "Can't read that chord line yet. The last good phrase is still armed. Try Am7, Fmaj7, Gsus4.";
        renderAndPublish();
        return;
    }
    progression = std::move(parsed);
    chordsAreCustom = true;
    storedText = text;
    errorText.clear();
    renderAndPublish();
}

void PluginProcessor::useStyleLine() {
    const int style = readChoice("style", c2m::kStyleCount);
    const int key = readChoice("key", 12);
    progression = c2m::styleProgression(c2m::styleAt(style), key);
    chordsAreCustom = false;
    storedText = c2m::formatProgression(progression);
    errorText.clear();
    ++textRevision;
    appliedStyle = style;
    appliedKey = key;
    renderAndPublish();
}

void PluginProcessor::generateNewProgression() {
    ++seed;
    if (seed == 0) seed = 1;
    const int key = readChoice("key", 12);
    progression = c2m::generateProgression(key, seed);
    chordsAreCustom = true;
    storedText = c2m::formatProgression(progression);
    errorText.clear();
    ++textRevision;
    appliedKey = key;
    renderAndPublish();
}

void PluginProcessor::setAudition(bool shouldPlay) {
    audition.store(shouldPlay);
}

std::vector<unsigned char> PluginProcessor::currentMidiFile() const {
    std::shared_ptr<const c2m::Phrase> local;
    {
        const juce::SpinLock::ScopedLockType guard(phraseLock);
        local = phrase;
    }
    if (local == nullptr || local->notes.empty()) return {};
    return c2m::renderSmf(*local);
}

void PluginProcessor::releaseAll(juce::MidiBuffer& midi, int sample) {
    bool any = false;
    for (auto& voice : voices) {
        if (!voice.on) continue;
        any = true;
        midi.addEvent(juce::MidiMessage::noteOff(1, voice.note), sample);
        voice.on = false;
    }
    if (any)
        midi.addEvent(juce::MidiMessage::allNotesOff(1), sample);
}

void PluginProcessor::noteOn(juce::MidiBuffer& midi, int sample, int note, int velocity, int durationSamples,
                            float gateBeats, float bpm, const c2m::Envelope& envelope) {
    note = std::clamp(note, 0, 127);
    velocity = std::clamp(velocity, 1, 127);
    durationSamples = std::max(1, durationSamples);
    for (auto& voice : voices) {
        if (voice.on && voice.note == note && voice.startSample <= sample) {
            midi.addEvent(juce::MidiMessage::noteOff(1, voice.note), sample);
            voice.on = false;
        }
    }
    Voice* slot = nullptr;
    for (auto& voice : voices) {
        if (!voice.on) {
            slot = &voice;
            break;
        }
    }
    if (slot == nullptr) {
        slot = &voices.front();
        for (auto& voice : voices)
            if (voice.samplesLeft < slot->samplesLeft)
                slot = &voice;
        midi.addEvent(juce::MidiMessage::noteOff(1, slot->note), sample);
    }
    slot->on = true;
    slot->note = note;
    slot->samplesLeft = durationSamples;
    slot->startSample = sample;
    slot->age = 0;
    slot->phase = 0.0f;
    slot->amp = static_cast<float>(velocity) / 127.0f;
    slot->gateBeats = gateBeats;
    slot->bpm = bpm;
    slot->envelope = envelope;
    midi.addEvent(juce::MidiMessage::noteOn(1, note, static_cast<juce::uint8>(velocity)), sample);
}

void PluginProcessor::playFrom(const c2m::Phrase& active, double startBeat, double beatSpan, double bpm, juce::MidiBuffer& midi) {
    if (active.lengthBeats <= 1.0e-6 || beatSpan <= 1.0e-8 || active.notes.empty()) return;
    const double length = active.lengthBeats;
    double position = std::fmod(startBeat, length);
    if (position < 0.0) position += length;

    double consumed = 0.0;
    const int blockSamples = currentBlockSamples;
    if (blockSamples <= 0) return;
    while (consumed < beatSpan - 1.0e-8) {
        if (position >= length) position = 0.0;
        const double room = std::max(0.0, length - position);
        const double take = std::min(room, beatSpan - consumed);
        if (take <= 1.0e-8) break;
        const int sample0 = static_cast<int>(std::floor(consumed / beatSpan * blockSamples));
        int sample1 = static_cast<int>(std::floor((consumed + take) / beatSpan * blockSamples));
        if (sample1 <= sample0) sample1 = std::min(blockSamples, sample0 + 1);
        if (sample1 > blockSamples) sample1 = blockSamples;
        const double safeBpm = bpm < 1.0 ? 120.0 : bpm;
        auto sampleFor = [&](double beats) {
            const double relative = (beats - position) / take;
            int sample = sample0 + static_cast<int>(std::floor(relative * (sample1 - sample0)));
            if (sample < sample0) sample = sample0;
            if (sample >= sample1) sample = sample1 - 1;
            return sample;
        };
        for (const auto& expression : active.expression) {
            if (expression.beats < position || expression.beats >= position + take) continue;
            midi.addEvent(juce::MidiMessage::controllerEvent(1, 11, std::clamp(expression.value, 0, 127)),
                          sampleFor(expression.beats));
        }
        for (const auto& event : active.notes) {
            if (event.startBeats < position || event.startBeats >= position + take) continue;
            const int duration = std::max(1, static_cast<int>(std::lround(event.lengthBeats * (60.0 / safeBpm) * sampleRate)));
            noteOn(midi, sampleFor(event.startBeats), event.midi, event.velocity, duration,
                   static_cast<float>(event.gateBeats), static_cast<float>(safeBpm), active.envelope);
        }
        consumed += take;
        position += take;
        if (position >= length - 1.0e-8) position = 0.0;
    }
}

void PluginProcessor::renderVoices(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();
    const bool playAudio = audition.load();
    bool any = false;
    for (const auto& voice : voices)
        any = any || voice.on;
    if (!any) return;

    for (int sample = 0; sample < numSamples; ++sample) {
        float mixed = 0.0f;
        for (auto& voice : voices) {
            if (!voice.on || sample < voice.startSample) continue;
            const double beats = voice.bpm <= 0.0f
                                     ? 0.0
                                     : (static_cast<double>(voice.age) / sampleRate) * (static_cast<double>(voice.bpm) / 60.0);
            const float shaped = static_cast<float>(c2m::envelopeAt(voice.envelope, voice.gateBeats, beats));
            if (playAudio && channels > 0) {
                const float frequency = 440.0f * std::pow(2.0f, (static_cast<float>(voice.note) - 69.0f) / 12.0f);
                mixed += std::sin(voice.phase * 6.2831853f) * voice.amp * shaped * 0.08f;
                voice.phase += frequency / static_cast<float>(sampleRate);
                if (voice.phase >= 1.0f) voice.phase -= 1.0f;
            }
            ++voice.age;
            --voice.samplesLeft;
            if (voice.samplesLeft <= 0) {
                midi.addEvent(juce::MidiMessage::noteOff(1, voice.note), sample);
                voice.on = false;
            }
        }
        if (playAudio) {
            mixed = std::clamp(mixed, -1.0f, 1.0f);
            for (int channel = 0; channel < channels; ++channel)
                buffer.addSample(channel, sample, mixed);
        }
    }
    for (auto& voice : voices)
        if (voice.on) voice.startSample = 0;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    midi.clear();
    currentBlockSamples = buffer.getNumSamples();
    if (currentBlockSamples <= 0 || sampleRate <= 0.0) return;

    std::shared_ptr<const c2m::Phrase> local;
    int token = 0;
    {
        const juce::SpinLock::ScopedLockType guard(phraseLock);
        local = phrase;
        token = phraseToken;
    }
    if (local == nullptr) return;

    if (token != armedToken) {
        releaseAll(midi, 0);
        armedToken = token;
        if (!seenHost) cursor = 0.0;
    }

    bool hostPlaying = false;
    double hostStart = 0.0;
    double hostBpm = static_cast<double>(local->bpm);
    if (auto* playHead = getPlayHead()) {
        if (const auto position = playHead->getPosition()) {
            hostPlaying = position->getIsPlaying();
            if (hostPlaying) {
                hostStart = position->getPpqPosition().orFallback(0.0);
                hostBpm = position->getBpm().orFallback(hostBpm);
            }
        }
    }
    if (hostBpm < 1.0) hostBpm = 120.0;
    const double hostSpan = (hostBpm / 60.0) * (static_cast<double>(currentBlockSamples) / sampleRate);

    if (hostPlaying) {
        if (seenHost) {
            const double gap = hostStart - expectedHost;
            if (gap < -0.002 || gap > 0.2)
                releaseAll(midi, 0);
        }
        seenHost = true;
        expectedHost = hostStart + hostSpan;
        if (local->lengthBeats > 0.0)
            cursor = std::fmod(expectedHost, local->lengthBeats);
        playFrom(*local, hostStart, hostSpan, hostBpm, midi);
    } else {
        if (seenHost)
            releaseAll(midi, 0);
        seenHost = false;
        if (audition.load() && local->lengthBeats > 0.0) {
            const double beats = (static_cast<double>(local->bpm) / 60.0) * (static_cast<double>(currentBlockSamples) / sampleRate);
            playFrom(*local, cursor, beats, local->bpm, midi);
            cursor = std::fmod(cursor + beats, local->lengthBeats);
            if (cursor < 0.0) cursor += local->lengthBeats;
        } else {
            releaseAll(midi, 0);
        }
    }

    renderVoices(buffer, midi);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
    publishProperties();
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr) return;
    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.isValid()) return;

    restoring = true;
    apvts.replaceState(tree);
    restoring = false;

    seed = static_cast<std::uint32_t>(static_cast<int>(tree.getProperty("seed", static_cast<int>(seed))));
    chordsAreCustom = static_cast<bool>(tree.getProperty("custom"));
    appliedStyle = readChoice("style", c2m::kStyleCount);
    appliedKey = readChoice("key", 12);
    const auto saved = tree.getProperty("chords").toString().toStdString();
    std::vector<c2m::Chord> parsed;
    if (c2m::parseProgression(saved, parsed)) {
        progression = std::move(parsed);
        storedText = saved;
    } else {
        progression = c2m::styleProgression(c2m::styleAt(appliedStyle), appliedKey);
        chordsAreCustom = false;
        storedText = c2m::formatProgression(progression);
    }
    errorText.clear();
    ++textRevision;
    renderAndPublish();
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
    return new PluginEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new PluginProcessor();
}
