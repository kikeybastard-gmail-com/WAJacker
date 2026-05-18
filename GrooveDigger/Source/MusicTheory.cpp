#include "MusicTheory.h"

std::vector<int> MusicTheory::getScaleIntervals(Scale scale)
{
    switch (scale)
    {
        case Scale::Major:      return { 0, 2, 4, 5, 7, 9, 11 };
        case Scale::Minor:      return { 0, 2, 3, 5, 7, 8, 10 };
        case Scale::Dorian:     return { 0, 2, 3, 5, 7, 9, 10 };
        case Scale::Mixolydian: return { 0, 2, 4, 5, 7, 9, 10 };
        case Scale::Pentatonic: return { 0, 2, 4, 7, 9 };
        case Scale::Blues:      return { 0, 3, 5, 6, 7, 10 };
        default:                return { 0, 2, 4, 5, 7, 9, 11 };
    }
}

std::vector<int> MusicTheory::getNotesInRange(int rootMidi, Scale scale, int octaveRange)
{
    auto intervals = getScaleIntervals(scale);
    std::vector<int> notes;
    notes.reserve(intervals.size() * (size_t)octaveRange);

    for (int oct = 0; oct < octaveRange; ++oct)
        for (int interval : intervals)
        {
            int note = rootMidi + oct * 12 + interval;
            if (note >= 21 && note <= 108)
                notes.push_back(note);
        }

    return notes;
}

const char* MusicTheory::getScaleName(int index)
{
    static const char* names[] = {
        "Major", "Minor", "Dorian", "Mixolydian", "Pentatonic", "Blues"
    };
    return (index >= 0 && index < 6) ? names[index] : "Major";
}

const char* MusicTheory::getKeyName(int index)
{
    static const char* names[] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };
    return (index >= 0 && index < 12) ? names[index] : "C";
}
