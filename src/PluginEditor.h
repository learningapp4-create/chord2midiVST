#pragma once

#include "PluginProcessor.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::DragAndDropContainer,
                     private juce::Timer {
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class DragButton : public juce::TextButton {
    public:
        std::function<void()> onDrag;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

    private:
        bool dragging = false;
    };

    void timerCallback() override;
    void refreshFromProcessor();
    juce::File tempMidiFile() const;

    PluginProcessor& processor;
    juce::LookAndFeel_V4 skin;

    juce::Label title;
    juce::Label version;
    juce::Label styleLabel;
    juce::Label keyLabel;
    juce::ComboBox styleBox;
    juce::ComboBox keyBox;
    juce::TextEditor character;
    juce::Label chordLabel;
    juce::TextEditor chords;
    juce::TextButton styleChords;
    juce::TextButton generate;
    juce::TextButton hear;
    DragButton dragMidi;
    juce::TextButton saveMidi;
    juce::Label tempoLabel;
    juce::Slider tempo;
    juce::Label feelLabel;
    juce::Label densityLabel;
    juce::Label rhythmLabel;
    juce::ComboBox feelBox;
    juce::ComboBox densityBox;
    juce::ComboBox rhythmBox;
    juce::TextEditor status;
    juce::TextEditor hint;

    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<ComboAttachment> styleAttachment;
    std::unique_ptr<ComboAttachment> keyAttachment;
    std::unique_ptr<ComboAttachment> feelAttachment;
    std::unique_ptr<ComboAttachment> densityAttachment;
    std::unique_ptr<ComboAttachment> rhythmAttachment;
    std::unique_ptr<SliderAttachment> tempoAttachment;
    std::shared_ptr<juce::FileChooser> chooser;

    int seenTextRevision = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
