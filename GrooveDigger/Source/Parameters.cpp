#include "Parameters.h"

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterChoice>(
        IDs::key, "Key",
        juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 0));

    p.push_back(std::make_unique<juce::AudioParameterChoice>(
        IDs::scale, "Scale",
        juce::StringArray{"Major","Minor","Dorian","Mixolydian","Pentatonic","Blues"}, 0));

    p.push_back(std::make_unique<juce::AudioParameterInt>(
        IDs::phraseLen, "Phrase Length (bars)", 1, 4, 2));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::density, "Density",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::syncopation, "Syncopation",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::restProb, "Rest Probability",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.2f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::swing, "Swing",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::variation, "Variation",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    p.push_back(std::make_unique<juce::AudioParameterInt>(
        IDs::octaveRange, "Octave Range", 1, 3, 1));

    p.push_back(std::make_unique<juce::AudioParameterInt>(
        IDs::rootOctave, "Root Octave", 2, 5, 3));

    p.push_back(std::make_unique<juce::AudioParameterBool>(
        IDs::regenerate, "Regenerate", false));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::sidechainThreshold, "Kick Threshold",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.15f));

    p.push_back(std::make_unique<juce::AudioParameterBool>(
        IDs::synthEnabled, "Synth Enabled", false));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthBlend, "Osc Blend",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthFilter, "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 2000.0f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthRes, "Filter Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthAttack, "Attack",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.0f, 0.5f), 0.01f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthDecay, "Decay",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.0f, 0.5f), 0.15f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthSustain, "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthRelease, "Release",
        juce::NormalisableRange<float>(0.001f, 4.0f, 0.0f, 0.5f), 0.1f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthGlide, "Glide",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    p.push_back(std::make_unique<juce::AudioParameterFloat>(
        IDs::synthDrive, "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    return { p.begin(), p.end() };
}
