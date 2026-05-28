#include "MonoSynth.h"
#include <cmath>
#include <algorithm>

MonoSynth::MonoSynth() {}

void MonoSynth::prepare(double sampleRate)
{
    sr = sampleRate;
    recalcEnv();
    filterState = 0.0f;
    filterFB    = 0.0f;
}

void MonoSynth::noteOn(int midiNote, float /*velocity*/, float glideTimeSec)
{
    targetFreq = midiToFreq(midiNote);
    if (envStage == Stage::Idle || glideTimeSec < 0.001f)
        currentFreq = targetFreq;

    // Glide: exponential approach per sample
    if (glideTimeSec < 0.001f)
        glideCoeff = 1.0f;
    else
        glideCoeff = 1.0f - std::exp(-1.0f / (float)(sr * glideTimeSec * 0.5));

    envStage = Stage::Attack;
}

void MonoSynth::noteOff()
{
    if (envStage != Stage::Idle)
        envStage = Stage::Release;
}

float MonoSynth::process()
{
    if (envStage == Stage::Idle)
        return 0.0f;

    // Glide
    currentFreq += (targetFreq - currentFreq) * glideCoeff;

    // Oscillator
    float inc   = currentFreq / (float)sr;
    phase      += inc;
    if (phase >= 1.0f) phase -= 1.0f;

    float sine = std::sin(phase * 6.28318530718f);
    float ramp = phase * 2.0f - 1.0f;
    float osc  = sine * (1.0f - oscBlend) + ramp * oscBlend;

    // ADSR
    switch (envStage)
    {
        case Stage::Attack:
            envVal += atkCoeff;
            if (envVal >= 1.0f) { envVal = 1.0f; envStage = Stage::Decay; }
            break;
        case Stage::Decay:
            envVal += (susLevel - envVal) * decCoeff;
            if (std::abs(envVal - susLevel) < 0.001f) { envVal = susLevel; envStage = Stage::Sustain; }
            break;
        case Stage::Sustain:
            envVal = susLevel;
            break;
        case Stage::Release:
            envVal *= relCoeff;
            if (envVal < 0.0001f) { envVal = 0.0f; envStage = Stage::Idle; }
            break;
        default: break;
    }

    osc *= envVal;

    // Drive
    if (drive > 0.0f)
        osc = softClip(osc * (1.0f + drive * 8.0f));

    // One-pole lowpass with resonance feedback
    float cutNorm = std::min(0.49f, filterCutHz / (float)sr);
    float b       = std::exp(-6.28318530718f * cutNorm);
    float a       = 1.0f - b;
    filterState   = a * (osc - filterRes * 3.5f * filterFB) + b * filterState;
    filterFB      = filterState;

    return filterState * 0.7f;
}

bool MonoSynth::isActive() const { return envStage != Stage::Idle; }

void MonoSynth::setOscBlend(float blend) { oscBlend = blend; }

void MonoSynth::setFilter(float cutoffHz, float res)
{
    filterCutHz = cutoffHz;
    filterRes   = res;
}

void MonoSynth::setADSR(float attack, float decay, float sustain, float release)
{
    atkTime  = attack;
    decTime  = decay;
    susLevel = sustain;
    relTime  = release;
    recalcEnv();
}

void MonoSynth::setDrive(float d) { drive = d; }

void MonoSynth::recalcEnv()
{
    float fsr = (float)sr;
    atkCoeff  = (atkTime > 0.0f) ? (1.0f / (atkTime * fsr)) : 1.0f;
    decCoeff  = (decTime > 0.0f) ? (1.0f - std::exp(-1.0f / (decTime * fsr * 0.5f))) : 1.0f;
    relCoeff  = (relTime > 0.0f) ? std::exp(-1.0f / (relTime * fsr)) : 0.0f;
}

float MonoSynth::midiToFreq(int note) const
{
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

float MonoSynth::softClip(float x)
{
    return x / (1.0f + std::abs(x));
}
