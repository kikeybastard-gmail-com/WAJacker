#include "PluginEditor.h"
#include "Parameters.h"

//==============================================================================
// Colours
namespace GDColours
{
    const juce::Colour bg        { 0xff0f0f1a };
    const juce::Colour panel     { 0xff1a1a2e };
    const juce::Colour panelAlt  { 0xff16213e };
    const juce::Colour accent    { 0xff6c63ff };
    const juce::Colour accentOn  { 0xffff6584 };
    const juce::Colour textDim   { 0xffaaaacc };
}

//==============================================================================
// KickGrid

KickGrid::KickGrid(juce::AudioProcessorValueTreeState& apvts)
{
    for (int i = 0; i < 16; ++i)
    {
        auto* btn = buttons.add(new juce::TextButton(juce::String(i + 1)));
        btn->setClickingTogglesState(true);
        btn->setColour(juce::TextButton::buttonColourId,   GDColours::panel);
        btn->setColour(juce::TextButton::buttonOnColourId, GDColours::accentOn);
        btn->setColour(juce::TextButton::textColourOffId,  GDColours::textDim);
        btn->setColour(juce::TextButton::textColourOnId,   juce::Colours::white);
        addAndMakeVisible(btn);

        attachments.add(new juce::AudioProcessorValueTreeState::ButtonAttachment(
            apvts, IDs::kickStep(i), *btn));
    }
}

void KickGrid::paint(juce::Graphics& g)
{
    g.fillAll(GDColours::panelAlt);
}

void KickGrid::resized()
{
    int w = getWidth() / 16;
    for (int i = 0; i < 16; ++i)
        buttons[i]->setBounds(i * w, 0, w, getHeight());
}

//==============================================================================
// GrooveDiggerEditor

void GrooveDiggerEditor::styleKnob(juce::Slider& s, juce::Label& lbl,
                                    const juce::String& text, juce::Component* parent)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 15);
    s.setColour(juce::Slider::rotarySliderFillColourId, GDColours::accent);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, GDColours::panel);
    s.setColour(juce::Slider::textBoxTextColourId, GDColours::textDim);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    parent->addAndMakeVisible(s);

    lbl.setText(text, juce::dontSendNotification);
    lbl.setFont(juce::Font(10.5f));
    lbl.setJustificationType(juce::Justification::centred);
    lbl.setColour(juce::Label::textColourId, GDColours::textDim);
    parent->addAndMakeVisible(lbl);
}

GrooveDiggerEditor::GrooveDiggerEditor(GrooveDiggerProcessor& p)
    : AudioProcessorEditor(&p), proc(p), kickGrid(p.apvts),
      keyAttach   (p.apvts, IDs::key,   keyBox),
      scaleAttach (p.apvts, IDs::scale, scaleBox),
      densityAtt  (p.apvts, IDs::density,     densityKnob),
      syncopAtt   (p.apvts, IDs::syncopation, syncopKnob),
      restAtt     (p.apvts, IDs::restProb,    restKnob),
      swingAtt    (p.apvts, IDs::swing,       swingKnob),
      varAtt      (p.apvts, IDs::variation,   varKnob),
      barsAtt     (p.apvts, IDs::phraseLen,   barsKnob),
      octRangeAtt (p.apvts, IDs::octaveRange, octRangeKnob),
      rootOctAtt  (p.apvts, IDs::rootOctave,  rootOctKnob),
      synthToggleAtt(p.apvts, IDs::synthEnabled, synthToggle),
      blendAtt  (p.apvts, IDs::synthBlend,   blendKnob),
      filterAtt (p.apvts, IDs::synthFilter,  filterKnob),
      resAtt    (p.apvts, IDs::synthRes,     resKnob),
      atkAtt    (p.apvts, IDs::synthAttack,  atkKnob),
      decAtt    (p.apvts, IDs::synthDecay,   decKnob),
      susAtt    (p.apvts, IDs::synthSustain, susKnob),
      relAtt    (p.apvts, IDs::synthRelease, relKnob),
      glideAtt  (p.apvts, IDs::synthGlide,   glideKnob),
      driveAtt  (p.apvts, IDs::synthDrive,   driveKnob)
{
    setSize(720, 520);

    // Title
    titleLabel.setText("GROOVE DIGGER", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(21.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, GDColours::accent);
    addAndMakeVisible(titleLabel);

    // Key
    keyLabel.setText("Key", juce::dontSendNotification);
    keyLabel.setFont(juce::Font(10.5f));
    keyLabel.setJustificationType(juce::Justification::centred);
    keyLabel.setColour(juce::Label::textColourId, GDColours::textDim);
    addAndMakeVisible(keyLabel);

    keyBox.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 1);
    keyBox.setColour(juce::ComboBox::backgroundColourId, GDColours::panel);
    keyBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    addAndMakeVisible(keyBox);

    // Scale
    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setFont(juce::Font(10.5f));
    scaleLabel.setJustificationType(juce::Justification::centred);
    scaleLabel.setColour(juce::Label::textColourId, GDColours::textDim);
    addAndMakeVisible(scaleLabel);

    scaleBox.addItemList({"Major","Minor","Dorian","Mixolydian","Pentatonic","Blues"}, 1);
    scaleBox.setColour(juce::ComboBox::backgroundColourId, GDColours::panel);
    scaleBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    addAndMakeVisible(scaleBox);

    // Regenerate button
    regenBtn.setButtonText("REGENERATE");
    regenBtn.setColour(juce::TextButton::buttonColourId,  GDColours::accent);
    regenBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    regenBtn.onClick = [this]{ proc.triggerRegenerate(); };
    addAndMakeVisible(regenBtn);

    // Main knobs
    styleKnob(densityKnob,  densityLbl,  "Density",   this);
    styleKnob(syncopKnob,   syncopLbl,   "Syncop",    this);
    styleKnob(restKnob,     restLbl,     "Rest %",    this);
    styleKnob(swingKnob,    swingLbl,    "Swing",     this);
    styleKnob(varKnob,      varLbl,      "Variation", this);
    styleKnob(barsKnob,     barsLbl,     "Bars",      this);
    styleKnob(octRangeKnob, octRangeLbl, "Oct Range", this);
    styleKnob(rootOctKnob,  rootOctLbl,  "Root Oct",  this);

    barsKnob.setRange(1, 4, 1);
    octRangeKnob.setRange(1, 3, 1);
    rootOctKnob.setRange(2, 5, 1);

    // Kick grid
    addAndMakeVisible(kickGrid);

    // Synth toggle
    synthToggle.setButtonText("Internal Synth");
    synthToggle.setColour(juce::ToggleButton::textColourId,      juce::Colours::white);
    synthToggle.setColour(juce::ToggleButton::tickColourId,       GDColours::accent);
    synthToggle.setColour(juce::ToggleButton::tickDisabledColourId, GDColours::textDim);
    addAndMakeVisible(synthToggle);

    // Synth knobs
    styleKnob(blendKnob,  blendLbl,  "Blend",   this);
    styleKnob(filterKnob, filterLbl, "Filter",  this);
    styleKnob(resKnob,    resLbl,    "Res",     this);
    styleKnob(atkKnob,    atkLbl,    "Attack",  this);
    styleKnob(decKnob,    decLbl,    "Decay",   this);
    styleKnob(susKnob,    susLbl,    "Sustain", this);
    styleKnob(relKnob,    relLbl,    "Release", this);
    styleKnob(glideKnob,  glideLbl,  "Glide",   this);
    styleKnob(driveKnob,  driveLbl,  "Drive",   this);

    refreshSynthVisibility();
    startTimerHz(10);
}

GrooveDiggerEditor::~GrooveDiggerEditor()
{
    stopTimer();
}

//==============================================================================
void GrooveDiggerEditor::timerCallback()
{
    refreshSynthVisibility();
}

void GrooveDiggerEditor::refreshSynthVisibility()
{
    bool on = proc.apvts.getRawParameterValue(IDs::synthEnabled)->load() > 0.5f;

    for (auto* c : std::initializer_list<juce::Component*>{
            &blendKnob,  &blendLbl,
            &filterKnob, &filterLbl,
            &resKnob,    &resLbl,
            &atkKnob,    &atkLbl,
            &decKnob,    &decLbl,
            &susKnob,    &susLbl,
            &relKnob,    &relLbl,
            &glideKnob,  &glideLbl,
            &driveKnob,  &driveLbl })
        c->setVisible(on);
}

//==============================================================================
void GrooveDiggerEditor::paint(juce::Graphics& g)
{
    g.fillAll(GDColours::bg);

    // Panel: main controls
    g.setColour(GDColours::panel);
    g.fillRoundedRectangle(8.0f,  48.0f, (float)getWidth() - 16.0f, 168.0f, 6.0f);

    // Panel: kick grid
    g.setColour(GDColours::panelAlt);
    g.fillRoundedRectangle(8.0f, 224.0f, (float)getWidth() - 16.0f, 50.0f, 6.0f);

    // Panel: synth
    g.setColour(GDColours::panel);
    g.fillRoundedRectangle(8.0f, 282.0f, (float)getWidth() - 16.0f, 228.0f, 6.0f);

    // Section labels
    g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.setColour(GDColours::textDim);
    g.drawText("KICK GRID", 18, 226, 70, 12, juce::Justification::left);
    g.drawText("SYNTH", 18, 284, 50, 12, juce::Justification::left);
}

void GrooveDiggerEditor::resized()
{
    const int W = getWidth();

    titleLabel.setBounds(0, 6, W, 32);

    // Key / Scale combos
    keyLabel.setBounds(14, 52, 70, 14);
    keyBox.setBounds(14, 66, 100, 24);
    scaleLabel.setBounds(120, 52, 100, 14);
    scaleBox.setBounds(120, 66, 140, 24);

    // Regenerate button
    regenBtn.setBounds(W - 134, 58, 124, 32);

    // 8 main knobs in one row
    const int kW  = 80;
    const int kH  = 72;
    const int kY  = 100;
    const int kX0 = 20;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int col)
    {
        int x = kX0 + col * kW;
        s.setBounds(x, kY, kW, kH - 16);
        l.setBounds(x, kY + kH - 16, kW, 14);
    };

    placeKnob(densityKnob,  densityLbl,  0);
    placeKnob(syncopKnob,   syncopLbl,   1);
    placeKnob(restKnob,     restLbl,     2);
    placeKnob(swingKnob,    swingLbl,    3);
    placeKnob(varKnob,      varLbl,      4);
    placeKnob(barsKnob,     barsLbl,     5);
    placeKnob(octRangeKnob, octRangeLbl, 6);
    placeKnob(rootOctKnob,  rootOctLbl,  7);

    // Kick grid
    kickGrid.setBounds(14, 236, W - 28, 34);

    // Synth toggle
    synthToggle.setBounds(14, 286, 150, 22);

    // 9 synth knobs in one row
    const int sW  = 72;
    const int sH  = 68;
    const int sY  = 316;
    const int sX0 = 18;

    auto placeSynth = [&](juce::Slider& s, juce::Label& l, int col)
    {
        int x = sX0 + col * sW;
        s.setBounds(x, sY, sW, sH - 14);
        l.setBounds(x, sY + sH - 14, sW, 14);
    };

    placeSynth(blendKnob,  blendLbl,  0);
    placeSynth(filterKnob, filterLbl, 1);
    placeSynth(resKnob,    resLbl,    2);
    placeSynth(atkKnob,    atkLbl,    3);
    placeSynth(decKnob,    decLbl,    4);
    placeSynth(susKnob,    susLbl,    5);
    placeSynth(relKnob,    relLbl,    6);
    placeSynth(glideKnob,  glideLbl,  7);
    placeSynth(driveKnob,  driveLbl,  8);
}
