#pragma once
#include <JuceHeader.h>

namespace IDs
{
    static const juce::String key          = "key";
    static const juce::String scale        = "scale";
    static const juce::String phraseLen    = "phraseLen";
    static const juce::String density      = "density";
    static const juce::String syncopation  = "syncopation";
    static const juce::String restProb     = "restProb";
    static const juce::String swing        = "swing";
    static const juce::String variation    = "variation";
    static const juce::String octaveRange  = "octaveRange";
    static const juce::String rootOctave   = "rootOctave";
    static const juce::String regenerate   = "regenerate";

    static const juce::String synthEnabled = "synthEnabled";
    static const juce::String synthBlend   = "synthBlend";
    static const juce::String synthFilter  = "synthFilter";
    static const juce::String synthRes     = "synthRes";
    static const juce::String synthAttack  = "synthAttack";
    static const juce::String synthDecay   = "synthDecay";
    static const juce::String synthSustain = "synthSustain";
    static const juce::String synthRelease = "synthRelease";
    static const juce::String synthGlide   = "synthGlide";
    static const juce::String synthDrive          = "synthDrive";

    static const juce::String sidechainThreshold  = "sidechainThreshold";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
