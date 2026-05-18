#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

GrooveDiggerProcessor::GrooveDiggerProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

GrooveDiggerProcessor::~GrooveDiggerProcessor() {}

//==============================================================================
void GrooveDiggerProcessor::prepareToPlay(double sampleRate, int)
{
    monoSynth.prepare(sampleRate);
    pendingNoteOffs.clear();
    lastNoteOn = -1;
    wasPlaying = false;
}

void GrooveDiggerProcessor::releaseResources() {}

//==============================================================================
void GrooveDiggerProcessor::maybeRegenerate()
{
    // Momentary trigger: detect rising edge on regenerate param
    float rv = apvts.getRawParameterValue(IDs::regenerate)->load();
    if (rv > 0.5f && lastRegenParam <= 0.5f)
        regenPending.store(true);
    lastRegenParam = rv;

    if (!regenPending.exchange(false))
        return;

    int   key       = (int)apvts.getRawParameterValue(IDs::key)->load();
    int   scale     = (int)apvts.getRawParameterValue(IDs::scale)->load();
    int   bars      = (int)apvts.getRawParameterValue(IDs::phraseLen)->load();
    float density   = apvts.getRawParameterValue(IDs::density)->load();
    float syncop    = apvts.getRawParameterValue(IDs::syncopation)->load();
    float restProb  = apvts.getRawParameterValue(IDs::restProb)->load();
    float variation = apvts.getRawParameterValue(IDs::variation)->load();
    int   octRange  = (int)apvts.getRawParameterValue(IDs::octaveRange)->load();
    int   rootOct   = (int)apvts.getRawParameterValue(IDs::rootOctave)->load();

    bool kickMask[16];
    for (int i = 0; i < 16; ++i)
        kickMask[i] = apvts.getRawParameterValue(IDs::kickStep(i))->load() > 0.5f;

    grooveEngine.regenerate(key, scale, bars * 16, density, syncop, restProb,
                            variation, octRange, rootOct, kickMask);
}

//==============================================================================
void GrooveDiggerProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midi)
{
    buffer.clear();
    maybeRegenerate();

    auto posOpt = getPlayHead() ? getPlayHead()->getPosition()
                                : juce::Optional<juce::AudioPlayHead::PositionInfo>{};

    if (!posOpt.hasValue() || !posOpt->getIsPlaying())
    {
        wasPlaying = false;
        midi.clear();
        return;
    }

    if (!wasPlaying)
    {
        wasPlaying = true;
        pendingNoteOffs.clear();
        lastNoteOn = -1;
    }

    midi.clear();
    scheduleEvents(midi, *posOpt, buffer.getNumSamples(), getSampleRate());

    // Internal synth audio
    bool synthOn = apvts.getRawParameterValue(IDs::synthEnabled)->load() > 0.5f;
    if (synthOn)
    {
        monoSynth.setOscBlend(apvts.getRawParameterValue(IDs::synthBlend)->load());
        monoSynth.setFilter(apvts.getRawParameterValue(IDs::synthFilter)->load(),
                            apvts.getRawParameterValue(IDs::synthRes)->load());
        monoSynth.setADSR(apvts.getRawParameterValue(IDs::synthAttack)->load(),
                          apvts.getRawParameterValue(IDs::synthDecay)->load(),
                          apvts.getRawParameterValue(IDs::synthSustain)->load(),
                          apvts.getRawParameterValue(IDs::synthRelease)->load());
        monoSynth.setDrive(apvts.getRawParameterValue(IDs::synthDrive)->load());

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
                data[s] = monoSynth.process();
        }
    }
}

//==============================================================================
void GrooveDiggerProcessor::scheduleEvents(juce::MidiBuffer& midi,
                                            const juce::AudioPlayHead::PositionInfo& pos,
                                            int numSamples,
                                            double sampleRate)
{
    const auto& steps   = grooveEngine.getSteps();
    int         numSteps = (int)steps.size();
    if (numSteps == 0) return;

    double bpm          = pos.getBpm().hasValue()          ? *pos.getBpm()          : 120.0;
    double ppqNow       = pos.getPpqPosition().hasValue()   ? *pos.getPpqPosition()  : 0.0;
    double ppqPerSample = bpm / 60.0 / sampleRate;
    double bufEndPPQ    = ppqNow + numSamples * ppqPerSample;
    double phrasePPQ    = numSteps * GrooveEngine::kSixteenthPPQ;
    float  swing        = apvts.getRawParameterValue(IDs::swing)->load();
    bool   synthOn      = apvts.getRawParameterValue(IDs::synthEnabled)->load() > 0.5f;
    float  glide        = apvts.getRawParameterValue(IDs::synthGlide)->load();

    auto sampleForPPQ = [&](double ppq) {
        return juce::jlimit(0, numSamples - 1,
                            (int)std::round((ppq - ppqNow) / ppqPerSample));
    };

    // ── Fire overdue / in-buffer note-offs ───────────────────────────────────
    for (auto it = pendingNoteOffs.begin(); it != pendingNoteOffs.end(); )
    {
        if (it->releasePPQ < bufEndPPQ)
        {
            int s = (it->releasePPQ >= ppqNow) ? sampleForPPQ(it->releasePPQ) : 0;
            midi.addEvent(juce::MidiMessage::noteOff(1, it->note), s);
            if (synthOn && it->note == lastNoteOn)
                monoSynth.noteOff();
            it = pendingNoteOffs.erase(it);
        }
        else
            ++it;
    }

    // ── Schedule note-ons for steps that land in this buffer ─────────────────
    long phraseStart = (long)std::floor(ppqNow / phrasePPQ);
    long phraseEnd   = (long)std::ceil(bufEndPPQ / phrasePPQ);

    for (long ph = phraseStart; ph <= phraseEnd; ++ph)
    {
        for (int si = 0; si < numSteps; ++si)
        {
            const auto& step = steps[si];
            if (!step.active) continue;

            double eventPPQ = ph * phrasePPQ + grooveEngine.getStepPositionPPQ(si, swing);
            if (eventPPQ < ppqNow || eventPPQ >= bufEndPPQ)
                continue;

            int sampleOff = sampleForPPQ(eventPPQ);

            // Kill the currently-held note before starting new one
            if (lastNoteOn >= 0)
            {
                auto it = std::find_if(pendingNoteOffs.begin(), pendingNoteOffs.end(),
                                       [&](const PendingNoteOff& n){ return n.note == lastNoteOn; });
                if (it != pendingNoteOffs.end())
                {
                    midi.addEvent(juce::MidiMessage::noteOff(1, lastNoteOn), sampleOff);
                    if (synthOn) monoSynth.noteOff();
                    pendingNoteOffs.erase(it);
                }
            }

            int vel = juce::jlimit(1, 127, (int)(step.velocity * 127.0f));
            midi.addEvent(juce::MidiMessage::noteOn(1, step.midiNote, (juce::uint8)vel), sampleOff);

            if (synthOn)
                monoSynth.noteOn(step.midiNote, step.velocity, glide * 0.4f);

            lastNoteOn = step.midiNote;
            pendingNoteOffs.push_back({ step.midiNote, eventPPQ + step.lengthPPQ });
        }
    }
}

//==============================================================================
void GrooveDiggerProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto xml   = state.createXml();
    copyXmlToBinary(*xml, destData);
}

void GrooveDiggerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
        regenPending.store(true);
    }
}

//==============================================================================
juce::AudioProcessorEditor* GrooveDiggerProcessor::createEditor()
{
    return new GrooveDiggerEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveDiggerProcessor();
}
