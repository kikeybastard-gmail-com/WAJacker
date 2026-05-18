#pragma once
#include <vector>
#include <random>
#include "MusicTheory.h"

struct GrooveStep
{
    bool  active      = false;
    int   midiNote    = 60;
    float velocity    = 0.8f;
    float lengthPPQ   = 0.25f;  // note length in quarter-note units
    bool  slide       = false;
};

class GrooveEngine
{
public:
    GrooveEngine();

    void regenerate(int key,
                    int scaleIndex,
                    int numSteps,        // phraseLen * 16
                    float density,
                    float syncopation,
                    float restProb,
                    float variation,
                    int octaveRange,
                    int rootOctave,
                    const bool* kickMask);

    const std::vector<GrooveStep>& getSteps() const { return steps; }
    int getNumSteps() const { return (int)steps.size(); }

    // PPQ position of step within one phrase cycle (0-based), swing applied.
    double getStepPositionPPQ(int stepIndex, float swing) const;

    static constexpr double kSixteenthPPQ = 0.25;

private:
    std::vector<GrooveStep> steps;
    std::mt19937 rng;
};
