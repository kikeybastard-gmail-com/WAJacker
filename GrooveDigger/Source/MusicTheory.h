#pragma once
#include <vector>

class MusicTheory
{
public:
    enum class Scale { Major = 0, Minor, Dorian, Mixolydian, Pentatonic, Blues };

    static std::vector<int> getScaleIntervals(Scale scale);

    // Returns MIDI note numbers for all scale tones within octaveRange octaves
    // starting from rootMidi.
    static std::vector<int> getNotesInRange(int rootMidi, Scale scale, int octaveRange);

    static const char* getScaleName(int index);
    static const char* getKeyName(int index);
};
