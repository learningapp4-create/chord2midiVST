#include "PluginEditor.h"

namespace {
void styleReadOnly(juce::TextEditor& editor, juce::Colour text, juce::Colour background) {
    editor.setMultiLine(true);
    editor.setReadOnly(true);
    editor.setCaretVisible(false);
    editor.setScrollbarsShown(false);
    editor.setInterceptsMouseClicks(false, false);
    editor.setColour(juce::TextEditor::textColourId, text);
    editor.setColour(juce::TextEditor::backgroundColourId, background);
    editor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    editor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
}

juce::Rectangle<int> column(juce::Rectangle<int>& row, int width) {
    auto area = row.removeFromLeft(width);
    if (row.getWidth() > 0) row.removeFromLeft(8);
    return area;
}
} // namespace

PluginEditor::PluginEditor(PluginProcessor& processorIn)
    : AudioProcessorEditor(&processorIn), processor(processorIn) {
    skin.setColour(juce::Label::textColourId, juce::Colour(0xfff3efe6));
    skin.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2e2b26));
    skin.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff3efe6));
    skin.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff5c554c));
    skin.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffe0a045));
    skin.setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff2e2b26));
    skin.setColour(juce::PopupMenu::textColourId, juce::Colour(0xfff3efe6));
    skin.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xffe0a045));
    skin.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xff1c1b19));
    skin.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a342c));
    skin.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe0a045));
    skin.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff3efe6));
    skin.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff1c1b19));
    skin.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xfff7f1e6));
    skin.setColour(juce::TextEditor::textColourId, juce::Colour(0xff1c1b19));
    skin.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xffc8b89a));
    skin.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffe0a045));
    skin.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe0a045));
    skin.setColour(juce::Slider::trackColourId, juce::Colour(0xff5c554c));
    skin.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2e2b26));
    skin.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff3efe6));
    skin.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff2e2b26));
    skin.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff5c554c));
    skin.setColour(juce::CaretComponent::caretColourId, juce::Colour(0xff1c1b19));
    setLookAndFeel(&skin);

    title.setText("Chord2MIDI", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(22.0f)));
    title.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(title);

    version.setText("0.2", juce::dontSendNotification);
    version.setJustificationType(juce::Justification::centredRight);
    version.setColour(juce::Label::textColourId, juce::Colour(0xffb7b0a4));
    addAndMakeVisible(version);

    styleLabel.setText("Style", juce::dontSendNotification);
    keyLabel.setText("Key", juce::dontSendNotification);
    addAndMakeVisible(styleLabel);
    addAndMakeVisible(keyLabel);
    for (int i = 0; i < c2m::kStyleCount; ++i)
        styleBox.addItem(c2m::styleAt(i).name, i + 1);
    for (int i = 0; i < 12; ++i)
        keyBox.addItem(c2m::keyName(i), i + 1);
    addAndMakeVisible(styleBox);
    addAndMakeVisible(keyBox);

    styleReadOnly(character, juce::Colour(0xfff3efe6), juce::Colours::transparentBlack);
    addAndMakeVisible(character);

    chordLabel.setText("Chords", juce::dontSendNotification);
    addAndMakeVisible(chordLabel);
    chords.setMultiLine(true);
    chords.setReturnKeyStartsNewLine(true);
    chords.setTextToShowWhenEmpty("Am7, Fmaj7, Csus4, Gadd9", juce::Colour(0xff8a8175));
    chords.onTextChange = [this] { processor.setCustomChords(chords.getText().toStdString()); };
    addAndMakeVisible(chords);

    styleChords.setButtonText("Style chords");
    styleChords.onClick = [this] { processor.useStyleLine(); };
    styleChords.setTooltip("Load this style's chord line in the selected key.");
    generate.setButtonText("New progression");
    generate.onClick = [this] { processor.generateNewProgression(); };
    generate.setTooltip("Build 8 chords in this key: four diatonic colours, then the same roots again.");
    hear.setButtonText("Hear");
    hear.setClickingTogglesState(true);
    hear.setToggleState(processor.isAudition(), juce::dontSendNotification);
    hear.onClick = [this] {
        processor.setAudition(hear.getToggleState());
        hear.setButtonText(hear.getToggleState() ? "Stop" : "Hear");
    };
    hear.setTooltip("Play a plain preview tone and send MIDI from the plugin clock.");
    dragMidi.setButtonText("Drag MIDI to a track");
    dragMidi.onDrag = [this] {
        const auto bytes = processor.currentMidiFile();
        if (bytes.empty()) {
            status.setText("Nothing to drag yet.", juce::dontSendNotification);
            return;
        }
        auto file = tempMidiFile();
        file.replaceWithData(bytes.data(), bytes.size());
        performExternalDragDropOfFiles(juce::StringArray(file.getFullPathName()), false, this);
    };
    dragMidi.setTooltip("Drag this button onto a DAW track. The file is one chord per bar.");
    saveMidi.setButtonText("Save MIDI…");
    saveMidi.onClick = [this] {
        const auto bytes = processor.currentMidiFile();
        if (bytes.empty()) {
            status.setText("Nothing to save yet.", juce::dontSendNotification);
            return;
        }
        chooser = std::make_shared<juce::FileChooser>(
            "Save Chord2MIDI phrase",
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("Chord2MIDI.mid"),
            "*.mid");
        const int flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;
        chooser->launchAsync(flags, [this, bytes](const juce::FileChooser& dialog) {
            const auto file = dialog.getResult();
            if (file == juce::File()) return;
            file.replaceWithData(bytes.data(), bytes.size());
        });
    };
    addAndMakeVisible(styleChords);
    addAndMakeVisible(generate);
    addAndMakeVisible(hear);
    addAndMakeVisible(dragMidi);
    addAndMakeVisible(saveMidi);

    tempoLabel.setText("Tempo", juce::dontSendNotification);
    addAndMakeVisible(tempoLabel);
    tempo.setSliderStyle(juce::Slider::LinearHorizontal);
    tempo.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);
    tempo.setTextValueSuffix(" BPM");
    addAndMakeVisible(tempo);

    volumeLabel.setText("Phrase volume", juce::dontSendNotification);
    addAndMakeVisible(volumeLabel);
    volume.setSliderStyle(juce::Slider::LinearHorizontal);
    volume.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);
    volume.setTextValueSuffix(" %");
    volume.setTooltip("Scales Hear and every note velocity and expression value in the MIDI file.");
    addAndMakeVisible(volume);

    auto addBeatSlider = [this](juce::Label& label, juce::Slider& slider, const juce::String& name, const juce::String& tip) {
        label.setText(name, juce::dontSendNotification);
        addAndMakeVisible(label);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 22);
        slider.setTextValueSuffix(" beats");
        slider.setTooltip(tip);
        addAndMakeVisible(slider);
    };
    addBeatSlider(attackLabel, attack, "Attack", "Beats from silence up to the peak of each note.");
    addBeatSlider(decayLabel, decay, "Decay", "Beats from the peak down to the sustain level.");
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    addAndMakeVisible(sustainLabel);
    sustain.setSliderStyle(juce::Slider::LinearHorizontal);
    sustain.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 22);
    sustain.setTextValueSuffix(" %");
    sustain.setTooltip("Level held after the decay, until the note's release.");
    addAndMakeVisible(sustain);
    addBeatSlider(releaseLabel, release, "Release", "Beats to fall to silence after the written note.");

    feelLabel.setText("Feel", juce::dontSendNotification);
    densityLabel.setText("Voicing", juce::dontSendNotification);
    rhythmLabel.setText("Rhythm", juce::dontSendNotification);
    addAndMakeVisible(feelLabel);
    addAndMakeVisible(densityLabel);
    addAndMakeVisible(rhythmLabel);
    feelBox.addItemList({"Straight", "Human", "Loose"}, 1);
    densityBox.addItemList({"Shell", "Close", "Full"}, 1);
    rhythmBox.addItemList({"Style", "Block", "Climb", "Fall", "Pairs"}, 1);
    feelBox.setTooltip("Straight is on the grid. Human and Loose lay some notes back a little.");
    densityBox.setTooltip("Shell keeps guide tones. Close is one register. Full adds a low bass and, on wide styles, an upper octave.");
    rhythmBox.setTooltip("Style keeps that style's pattern. Block, Climb, Fall, and Pairs replace it.");
    addAndMakeVisible(feelBox);
    addAndMakeVisible(densityBox);
    addAndMakeVisible(rhythmBox);

    styleReadOnly(status, juce::Colour(0xfff3efe6), juce::Colours::transparentBlack);
    styleReadOnly(hint, juce::Colour(0xffb7b0a4), juce::Colours::transparentBlack);
    hint.setText("Hear uses the tempo, the note envelope, and phrase volume. The MIDI file stores the accent in velocity and the envelope in CC11 expression, both scaled by phrase volume. In a DAW, press play to send that MIDI. Drag or save a .mid file. Changing style loads its chords, tempo, and feel.");
    addAndMakeVisible(status);
    addAndMakeVisible(hint);

    styleAttachment = std::make_unique<ComboAttachment>(processor.apvts, "style", styleBox);
    keyAttachment = std::make_unique<ComboAttachment>(processor.apvts, "key", keyBox);
    feelAttachment = std::make_unique<ComboAttachment>(processor.apvts, "feel", feelBox);
    densityAttachment = std::make_unique<ComboAttachment>(processor.apvts, "density", densityBox);
    rhythmAttachment = std::make_unique<ComboAttachment>(processor.apvts, "rhythm", rhythmBox);
    tempoAttachment = std::make_unique<SliderAttachment>(processor.apvts, "tempo", tempo);
    volumeAttachment = std::make_unique<SliderAttachment>(processor.apvts, "volume", volume);
    attackAttachment = std::make_unique<SliderAttachment>(processor.apvts, "attack", attack);
    decayAttachment = std::make_unique<SliderAttachment>(processor.apvts, "decay", decay);
    sustainAttachment = std::make_unique<SliderAttachment>(processor.apvts, "sustain", sustain);
    releaseAttachment = std::make_unique<SliderAttachment>(processor.apvts, "release", release);

    setSize(780, 860);
    setResizable(false, false);
    refreshFromProcessor();
    startTimerHz(12);
}

PluginEditor::~PluginEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void PluginEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff1c1b19));
    graphics.setColour(juce::Colour(0xffe0a045));
    graphics.fillRect(0, 0, getWidth(), 4);
}

void PluginEditor::resized() {
    auto area = getLocalBounds().reduced(18);
    area.removeFromTop(6);
    auto header = area.removeFromTop(32);
    version.setBounds(header.removeFromRight(48));
    title.setBounds(header);
    area.removeFromTop(8);

    auto styleRow = area.removeFromTop(26);
    styleLabel.setBounds(styleRow.removeFromLeft(48));
    keyBox.setBounds(styleRow.removeFromRight(110));
    keyLabel.setBounds(styleRow.removeFromRight(36));
    styleRow.removeFromRight(8);
    styleBox.setBounds(styleRow);
    area.removeFromTop(6);
    character.setBounds(area.removeFromTop(44));
    area.removeFromTop(8);

    chordLabel.setBounds(area.removeFromTop(18));
    chords.setBounds(area.removeFromTop(72));
    area.removeFromTop(8);

    auto buttons = area.removeFromTop(32);
    const int third = (buttons.getWidth() - 16) / 3;
    styleChords.setBounds(column(buttons, third));
    generate.setBounds(column(buttons, third));
    hear.setBounds(buttons);
    area.removeFromTop(8);
    auto exportRow = area.removeFromTop(32);
    saveMidi.setBounds(exportRow.removeFromRight(150));
    exportRow.removeFromRight(8);
    dragMidi.setBounds(exportRow);
    area.removeFromTop(12);

    auto tempoRow = area.removeFromTop(32);
    tempoLabel.setBounds(tempoRow.removeFromLeft(56));
    tempo.setBounds(tempoRow);
    area.removeFromTop(10);

    auto labels = area.removeFromTop(18);
    auto boxes = area.removeFromTop(28);
    const int columnWidth = (labels.getWidth() - 16) / 3;
    feelLabel.setBounds(column(labels, columnWidth));
    densityLabel.setBounds(column(labels, columnWidth));
    rhythmLabel.setBounds(labels);
    feelBox.setBounds(column(boxes, columnWidth));
    densityBox.setBounds(column(boxes, columnWidth));
    rhythmBox.setBounds(boxes);
    area.removeFromTop(10);

    auto volumeRow = area.removeFromTop(32);
    volumeLabel.setBounds(volumeRow.removeFromLeft(110));
    volume.setBounds(volumeRow);
    area.removeFromTop(8);

    auto envLabels = area.removeFromTop(18);
    auto envSliders = area.removeFromTop(36);
    const int envWidth = (envLabels.getWidth() - 24) / 4;
    attackLabel.setBounds(column(envLabels, envWidth));
    decayLabel.setBounds(column(envLabels, envWidth));
    sustainLabel.setBounds(column(envLabels, envWidth));
    releaseLabel.setBounds(envLabels);
    attack.setBounds(column(envSliders, envWidth));
    decay.setBounds(column(envSliders, envWidth));
    sustain.setBounds(column(envSliders, envWidth));
    release.setBounds(envSliders);
    area.removeFromTop(10);

    status.setBounds(area.removeFromTop(56));
    area.removeFromTop(4);
    hint.setBounds(area.removeFromTop(72));
}

void PluginEditor::timerCallback() {
    refreshFromProcessor();
}

void PluginEditor::refreshFromProcessor() {
    const auto snapshot = processor.model();
    if (snapshot.textRevision != seenTextRevision) {
        seenTextRevision = snapshot.textRevision;
        chords.setText(snapshot.chords, juce::dontSendNotification);
    }
    character.setText(snapshot.character, juce::dontSendNotification);
    status.setText(snapshot.status, juce::dontSendNotification);
}

juce::File PluginEditor::tempMidiFile() const {
    return juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("Chord2MIDI-phrase.mid");
}

void PluginEditor::DragButton::mouseDown(const juce::MouseEvent& event) {
    dragging = false;
    juce::TextButton::mouseDown(event);
}

void PluginEditor::DragButton::mouseDrag(const juce::MouseEvent& event) {
    if (!dragging && event.getDistanceFromDragStart() > 8) {
        dragging = true;
        if (onDrag) onDrag();
    }
    juce::TextButton::mouseDrag(event);
}

void PluginEditor::DragButton::mouseUp(const juce::MouseEvent& event) {
    dragging = false;
    juce::TextButton::mouseUp(event);
}
