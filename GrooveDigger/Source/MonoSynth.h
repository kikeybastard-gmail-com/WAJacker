#pragma once
#include <cmath>

class MonoSynth
{
public:
    MonoSynth();

    void prepare(double sampleRate);

    void noteOn(int midiNote, float velocity, float glideTimeSec);
    void noteOff();

    float process();

    void setOscBlend(float blend);              // 0 = sine, 1 = ramp/saw
    void setFilter(float cutoffHz, float res);  // res 0-1
    void setADSR(float attack, float decay, float sustain, float release);
    void setDrive(float drive);                 // 0-1

    bool isActive() const;

private:
    double sr = 44100.0;

    // Oscillator
    float phase       = 0.0f;
    float currentFreq = 440.0f;
    float targetFreq  = 440.0f;
    float glideCoeff  = 1.0f;
    float oscBlend    = 0.5f;

    // Filter (one-pole with resonance feedback)
    float filterCutHz = 2000.0f;
    float filterRes   = 0.3f;
    float filterState = 0.0f;
    float filterFB    = 0.0f;

    // ADSR
    enum class Stage { Idle, Attack, Decay, Sustain, Release };
    Stage envStage    = Stage::Idle;
    float envVal      = 0.0f;
    float atkCoeff    = 0.0f;
    float decCoeff    = 0.0f;
    float susLevel    = 0.7f;
    float relCoeff    = 0.0f;

    // Stored times (seconds) for recalculation when SR changes
    float atkTime = 0.01f;
    float decTime = 0.15f;
    float relTime = 0.10f;

    float drive = 0.0f;

    void recalcEnv();
    float midiToFreq(int note) const;
    static float softClip(float x);
};
