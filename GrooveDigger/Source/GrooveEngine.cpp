#include "GrooveEngine.h"
#include <cmath>

GrooveEngine::GrooveEngine()
    : rng(std::random_device{}())
{
}

double GrooveEngine::getStepPositionPPQ(int stepIndex, float swing) const
{
    double base = stepIndex * kSixteenthPPQ;
    // Odd 16th-note positions are the "off-beat" 8th-note subdivisions.
    // swing 0.5 = straight, 1.0 = full triplet swing (~0.667 of an 8th note).
    if (stepIndex % 2 == 1)
    {
        double swingOffset = (swing - 0.5) * 2.0 * kSixteenthPPQ * 0.667;
        base += swingOffset;
    }
    return base;
}

void GrooveEngine::regenerate(int key,
                               int scaleIndex,
                               int numSteps,
                               float density,
                               float syncopation,
                               float restProb,
                               float variation,
                               int octaveRange,
                               int rootOctave,
                               const bool* kickMask)
{
    auto scale      = static_cast<MusicTheory::Scale>(scaleIndex);
    int  rootMidi   = (rootOctave + 1) * 12 + key;
    auto available  = MusicTheory::getNotesInRange(rootMidi, scale, octaveRange);

    if (available.empty())
        available = { rootMidi };

    steps.resize(numSteps);

    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    std::uniform_int_distribution<int>    noteIdx(0, (int)available.size() - 1);

    for (int i = 0; i < numSteps; ++i)
    {
        auto& s  = steps[i];
        s.active = false;
        s.slide  = false;

        bool isKick = kickMask[i % 16];

        // Probability by rhythmic position
        float p;
        if      (i % 4 == 0) p = density;                                    // downbeat
        else if (i % 2 == 0) p = density * (0.5f + syncopation * 0.5f);     // upbeat
        else                  p = density * syncopation * 0.6f;               // 16th

        if (isKick) p *= (1.0f - syncopation * 0.7f);
        if (prob(rng) < restProb) p = 0.0f;

        s.active = (prob(rng) < p);

        if (s.active)
        {
            s.midiNote = available[noteIdx(rng)];
            s.velocity  = 0.5f + prob(rng) * 0.45f;

            // Heavier accent on downbeats
            if (i % 4 == 0)
                s.velocity = std::min(1.0f, s.velocity + 0.15f);

            // Longer notes on heavier beats
            if      (i % 4 == 0) s.lengthPPQ = 0.18f + prob(rng) * 0.12f;
            else if (i % 2 == 0) s.lengthPPQ = 0.12f + prob(rng) * 0.08f;
            else                  s.lengthPPQ = 0.06f + prob(rng) * 0.06f;

            s.slide = (prob(rng) < variation * 0.3f);
        }
    }
}
