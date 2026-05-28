#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class SidechainMeter : public juce::Component
{
public:
    SidechainMeter() = default;

    void setMask(uint16_t mask)
    {
        if (mask != currentMask)
        {
            currentMask = mask;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    uint16_t currentMask = 0;
};

//==============================================================================
class GrooveDiggerEditor : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    explicit GrooveDiggerEditor(GrooveDiggerProcessor&);
    ~GrooveDiggerEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GrooveDiggerProcessor& proc;

    // ── Header ───────────────────────────────────────────────────────────────
    juce::Label titleLabel;

    // ── Key / Scale ──────────────────────────────────────────────────────────
    juce::Label    keyLabel, scaleLabel;
    juce::ComboBox keyBox, scaleBox;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment keyAttach, scaleAttach;

    // ── Main knobs (8) ───────────────────────────────────────────────────────
    juce::Slider densityKnob, syncopKnob, restKnob, swingKnob,
                 varKnob, barsKnob, octRangeKnob, rootOctKnob;
    juce::Label  densityLbl, syncopLbl, restLbl, swingLbl,
                 varLbl, barsLbl, octRangeLbl, rootOctLbl;

    juce::AudioProcessorValueTreeState::SliderAttachment
        densityAtt, syncopAtt, restAtt, swingAtt,
        varAtt, barsAtt, octRangeAtt, rootOctAtt;

    // ── Regenerate ───────────────────────────────────────────────────────────
    juce::TextButton regenBtn;

    // ── Sidechain kick section ───────────────────────────────────────────────
    SidechainMeter sidechainMeter;
    juce::Slider   threshKnob;
    juce::Label    threshLbl;
    juce::AudioProcessorValueTreeState::SliderAttachment threshAtt;

    // ── Synth section ────────────────────────────────────────────────────────
    juce::ToggleButton synthToggle;
    juce::AudioProcessorValueTreeState::ButtonAttachment synthToggleAtt;

    juce::Slider blendKnob, filterKnob, resKnob,
                 atkKnob, decKnob, susKnob, relKnob,
                 glideKnob, driveKnob;
    juce::Label  blendLbl, filterLbl, resLbl,
                 atkLbl, decLbl, susLbl, relLbl,
                 glideLbl, driveLbl;

    juce::AudioProcessorValueTreeState::SliderAttachment
        blendAtt, filterAtt, resAtt,
        atkAtt, decAtt, susAtt, relAtt,
        glideAtt, driveAtt;

    // ── Helpers ──────────────────────────────────────────────────────────────
    void timerCallback() override;
    void refreshSynthVisibility();

    static void styleKnob(juce::Slider& s, juce::Label& lbl,
                          const juce::String& text, juce::Component* parent);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveDiggerEditor)
};
