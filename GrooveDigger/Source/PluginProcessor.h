#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <vector>
#include "Parameters.h"
#include "GrooveEngine.h"
#include "MonoSynth.h"

class GrooveDiggerProcessor : public juce::AudioProcessor
{
public:
    GrooveDiggerProcessor();
    ~GrooveDiggerProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Groove Digger"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms() override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    GrooveEngine grooveEngine;

    // Called from the editor's Regenerate button (message thread).
    void triggerRegenerate() { regenPending.store(true); }

private:
    struct PendingNoteOff
    {
        int    note;
        double releasePPQ;  // absolute host PPQ when note-off fires
    };

    MonoSynth              monoSynth;
    std::vector<PendingNoteOff> pendingNoteOffs;
    int                    lastNoteOn   = -1;
    bool                   wasPlaying   = false;

    std::atomic<bool>      regenPending { true };  // generate on first block
    float                  lastRegenParam = 0.0f;

    void maybeRegenerate();
    void scheduleEvents(juce::MidiBuffer& midi,
                        const juce::AudioPlayHead::PositionInfo& pos,
                        int numSamples,
                        double sampleRate);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveDiggerProcessor)
};
