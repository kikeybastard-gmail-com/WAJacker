#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

GrooveDiggerProcessor::GrooveDiggerProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
          .withInput ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

bool GrooveDiggerProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Must have stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Sidechain can be stereo, mono, or disabled
    const auto& sc = layouts.getChannelSet(true, 1);
    if (!sc.isDisabled() &&
        sc != juce::AudioChannelSet::mono() &&
        sc != juce::AudioChannelSet::stereo())
        return false;

    return true;
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

    uint16_t bits = detectedKickBits.load(std::memory_order_relaxed);
    bool kickMask[16];
    for (int i = 0; i < 16; ++i)
        kickMask[i] = (bits >> i) & 1;

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

    // ── Sidechain kick detection (runs regardless of playback state) ─────────
    {
        auto sc = getBusBuffer(buffer, true, 1);
        const double sr = getSampleRate();

        // Compute peak over block from first sidechain channel (if available)
        float peak = 0.0f;
        if (sc.getNumChannels() > 0)
        {
            const float* scData = sc.getReadPointer(0);
            for (int s = 0; s < sc.getNumSamples(); ++s)
                peak = std::max(peak, std::abs(scData[s]));
        }

        // Envelope follower: attack ~2ms, release ~80ms
        const float attackCoeff  = (sr > 0.0) ? std::exp(-1.0 / (0.002 * sr)) : 0.0f;
        const float releaseCoeff = (sr > 0.0) ? std::exp(-1.0 / (0.080 * sr)) : 0.0f;

        if (peak > sidechainEnv)
            sidechainEnv = attackCoeff  * sidechainEnv + (1.0f - attackCoeff)  * peak;
        else
            sidechainEnv = releaseCoeff * sidechainEnv + (1.0f - releaseCoeff) * peak;

        float threshold = apvts.getRawParameterValue(IDs::sidechainThreshold)->load();
        bool kickNow = (sidechainEnv > threshold);

        // Rising-edge detection: only act when playback is active and PPQ is valid
        if (posOpt.hasValue() && posOpt->getPpqPosition().hasValue())
        {
            double ppqNow = *posOpt->getPpqPosition();

            // Clear mask at start of each new bar
            if (lastBarPPQ >= 0.0 &&
                std::floor(ppqNow / 4.0) != std::floor(lastBarPPQ / 4.0))
            {
                detectedKickBits.store(0, std::memory_order_relaxed);
            }

            if (kickNow && !prevKickState)
            {
                int step = (int)(std::fmod(ppqNow, 4.0) / 0.25) % 16;
                detectedKickBits.fetch_or((uint16_t)(1u << step), std::memory_order_relaxed);
            }

            lastBarPPQ = ppqNow;
        }

        prevKickState = kickNow;
    }

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
