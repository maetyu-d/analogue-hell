#include "PluginEditor.h"
#include <cmath>

namespace
{
struct Theme
{
    juce::Colour accent;
    juce::Colour accentSoft;
    juce::Colour panelBg;
    juce::Colour panelEdge;
    juce::Colour textBright;
    juce::Colour textDim;
};

Theme getTheme(int modelId)
{
    switch (modelId)
    {
        case 1: return { juce::Colour::fromRGB(255, 140, 90), juce::Colour::fromRGB(150, 90, 68), juce::Colour::fromRGB(40, 14, 12), juce::Colour::fromRGB(170, 78, 52), juce::Colour::fromRGB(245, 230, 220), juce::Colour::fromRGB(187, 160, 150) };
        case 2: return { juce::Colour::fromRGB(242, 170, 75), juce::Colour::fromRGB(148, 99, 54), juce::Colour::fromRGB(42, 23, 10), juce::Colour::fromRGB(166, 108, 42), juce::Colour::fromRGB(248, 235, 210), juce::Colour::fromRGB(190, 172, 140) };
        case 3: return { juce::Colour::fromRGB(247, 130, 120), juce::Colour::fromRGB(132, 72, 74), juce::Colour::fromRGB(38, 11, 14), juce::Colour::fromRGB(154, 72, 78), juce::Colour::fromRGB(255, 228, 230), juce::Colour::fromRGB(193, 150, 152) };
        case 4: return { juce::Colour::fromRGB(236, 122, 90), juce::Colour::fromRGB(126, 72, 58), juce::Colour::fromRGB(34, 15, 10), juce::Colour::fromRGB(151, 70, 53), juce::Colour::fromRGB(250, 232, 224), juce::Colour::fromRGB(182, 155, 145) };
        case 5: return { juce::Colour::fromRGB(230, 90, 85), juce::Colour::fromRGB(127, 58, 56), juce::Colour::fromRGB(32, 10, 11), juce::Colour::fromRGB(152, 62, 61), juce::Colour::fromRGB(255, 224, 222), juce::Colour::fromRGB(190, 146, 146) };
        case 6: return { juce::Colour::fromRGB(240, 155, 95), juce::Colour::fromRGB(132, 84, 62), juce::Colour::fromRGB(35, 17, 10), juce::Colour::fromRGB(160, 91, 54), juce::Colour::fromRGB(248, 233, 220), juce::Colour::fromRGB(186, 160, 145) };
        case 7: return { juce::Colour::fromRGB(165, 206, 124), juce::Colour::fromRGB(95, 122, 74), juce::Colour::fromRGB(20, 30, 18), juce::Colour::fromRGB(98, 142, 74), juce::Colour::fromRGB(226, 240, 218), juce::Colour::fromRGB(162, 184, 151) };
        case 8: return { juce::Colour::fromRGB(130, 214, 186), juce::Colour::fromRGB(74, 126, 110), juce::Colour::fromRGB(14, 30, 24), juce::Colour::fromRGB(72, 147, 121), juce::Colour::fromRGB(220, 242, 234), juce::Colour::fromRGB(145, 185, 172) };
        case 9: return { juce::Colour::fromRGB(118, 188, 232), juce::Colour::fromRGB(71, 106, 136), juce::Colour::fromRGB(14, 23, 34), juce::Colour::fromRGB(68, 117, 165), juce::Colour::fromRGB(220, 232, 245), juce::Colour::fromRGB(148, 171, 196) };
        default: return { juce::Colour::fromRGB(187, 155, 239), juce::Colour::fromRGB(100, 86, 133), juce::Colour::fromRGB(22, 18, 34), juce::Colour::fromRGB(108, 92, 164), juce::Colour::fromRGB(234, 226, 250), juce::Colour::fromRGB(168, 158, 194) };
    }
}
}

class AnalogueHellAudioProcessorEditor::HellLookAndFeel : public juce::LookAndFeel_V4
{
public:
    explicit HellLookAndFeel(Theme t) : theme(t) {}

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override
    {
        // Keep rotary graphics in the upper region so they align with the knob body, not the text box.
        auto bounds = juce::Rectangle<float>(x, y, (float) width, (float) height * 0.82f).reduced(8.0f, 6.0f);
        const float ovalH = juce::jmin(123.0f, bounds.getHeight() * 0.84f);
        auto body = juce::Rectangle<float>(bounds.getX(), bounds.getCentreY() - ovalH * 0.50f, bounds.getWidth(), ovalH);

        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(body.translated(0.0f, 3.0f), body.getHeight() * 0.5f);

        g.setColour(theme.accentSoft.withMultipliedBrightness(0.38f));
        g.fillRoundedRectangle(body, body.getHeight() * 0.5f);

        auto inner = body.reduced(body.getWidth() * 0.12f, body.getHeight() * 0.22f);
        g.setColour(theme.accent.withAlpha(0.24f));
        g.fillRoundedRectangle(inner, inner.getHeight() * 0.5f);

        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawRoundedRectangle(body.reduced(1.5f), body.getHeight() * 0.5f, 1.2f);

        const float knobRadius = inner.getHeight() * 1.02f;
        auto dial = juce::Rectangle<float>(inner.getCentreX() - knobRadius, inner.getCentreY() - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        juce::Path arc;
        arc.addCentredArc(dial.getCentreX(), dial.getCentreY(), knobRadius * 0.82f, knobRadius * 0.82f, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(theme.textBright.withAlpha(0.14f));
        g.strokePath(arc, juce::PathStrokeType(4.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        arc.clear();
        arc.addCentredArc(dial.getCentreX(), dial.getCentreY(), knobRadius * 0.82f, knobRadius * 0.82f, 0.0f,
                          rotaryStartAngle, angle, true);

        g.setColour(theme.accent);
        g.strokePath(arc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Line<float> needle(
            juce::Point<float>(dial.getCentreX(), dial.getCentreY()),
            juce::Point<float>(dial.getCentreX() + std::cos(angle) * knobRadius * 0.68f,
                               dial.getCentreY() + std::sin(angle) * knobRadius * 0.68f));

        g.setColour(theme.textBright.withAlpha(0.85f));
        g.drawLine(needle, 2.1f);
    }

private:
    Theme theme;
};

struct AnalogueHellAudioProcessorEditor::Preset
{
    juce::String name;
    float drive;
    float mix;
    float depth;
    float rate;
    float character;
    float noise;
    float unstable;
    float stereo;
};

AnalogueHellAudioProcessorEditor::AnalogueHellAudioProcessorEditor(AnalogueHellAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      sliders { &drive, &depth, &character, &rate, &mix, &noise, &unstable, &stereo },
      labels { &driveL, &depthL, &characterL, &rateL, &mixL, &noiseL, &unstableL, &stereoL }
{
    setSize(980, 680);

    const auto theme = getTheme(processorRef.getModelId());
    lookAndFeel = std::make_unique<HellLookAndFeel>(theme);
    setLookAndFeel(lookAndFeel.get());

    pluginTitle.setText(juce::String(AH_PLUGIN_NAME), juce::dontSendNotification);
    pluginTitle.setJustificationType(juce::Justification::centredLeft);
    pluginTitle.setFont(juce::FontOptions(36.0f, juce::Font::bold));
    pluginTitle.setColour(juce::Label::textColourId, theme.textBright);
    addAndMakeVisible(pluginTitle);

    pluginSubtitle.setText("Analogue malfunction engine", juce::dontSendNotification);
    pluginSubtitle.setJustificationType(juce::Justification::centredLeft);
    pluginSubtitle.setFont(juce::FontOptions(16.0f, juce::Font::plain));
    pluginSubtitle.setColour(juce::Label::textColourId, theme.textDim);
    addAndMakeVisible(pluginSubtitle);

    sectionMacro.setText("MACRO ARRAY", juce::dontSendNotification);
    sectionMacro.setJustificationType(juce::Justification::centredLeft);
    sectionMacro.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    sectionMacro.setColour(juce::Label::textColourId, theme.accent);
    addAndMakeVisible(sectionMacro);

    sectionGlobal.setText("GLOBAL", juce::dontSendNotification);
    sectionGlobal.setJustificationType(juce::Justification::centredLeft);
    sectionGlobal.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    sectionGlobal.setColour(juce::Label::textColourId, theme.accent);
    addAndMakeVisible(sectionGlobal);

    presetBox.setColour(juce::ComboBox::backgroundColourId, theme.panelBg.brighter(0.1f));
    presetBox.setColour(juce::ComboBox::outlineColourId, theme.panelEdge);
    presetBox.setColour(juce::ComboBox::textColourId, theme.textBright);
    presetBox.setColour(juce::ComboBox::arrowColourId, theme.textBright.withAlpha(0.8f));
    presetBox.onChange = [this]
    {
        if (presetChangeInternal)
            return;
        applyPreset(presetBox.getSelectedItemIndex());
    };
    addAndMakeVisible(presetBox);

    for (auto* b : { &prevPreset, &nextPreset })
    {
        b->setColour(juce::TextButton::buttonColourId, theme.panelBg.brighter(0.25f));
        b->setColour(juce::TextButton::textColourOffId, theme.textBright.withAlpha(0.95f));
        b->setColour(juce::TextButton::buttonOnColourId, theme.accent.withAlpha(0.45f));
        b->setColour(juce::TextButton::textColourOnId, theme.textBright);
        b->setColour(juce::TextButton::buttonColourId, theme.panelBg.brighter(0.22f));
        b->setConnectedEdges(juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
        b->setTriggeredOnMouseDown(false);
        addAndMakeVisible(*b);
    }

    prevPreset.onClick = [this] { selectPresetRelative(-1); };
    nextPreset.onClick = [this] { selectPresetRelative(1); };

    addSlider(drive, driveL, "drive", "Drive");
    addSlider(depth, depthL, "depth", "Depth");
    addSlider(character, characterL, "character", "Character");
    addSlider(rate, rateL, "rate", "Rate");

    addSlider(mix, mixL, "mix", "Mix");
    addSlider(noise, noiseL, "noise", "Noise");
    addSlider(unstable, unstableL, "unstable", "Unstable");
    addSlider(stereo, stereoL, "stereo", "Stereo");

    if (processorRef.getModelId() == 7)
    {
        noiseL.setText("Cutoff", juce::dontSendNotification);
        unstableL.setText("Resonance", juce::dontSendNotification);
    }

    buildPresetMenu();
    presetChangeInternal = true;
    presetBox.setSelectedId(1, juce::sendNotificationSync);
    presetChangeInternal = false;
    applyPreset(0);
}

AnalogueHellAudioProcessorEditor::~AnalogueHellAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AnalogueHellAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto theme = getTheme(processorRef.getModelId());
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bg(juce::Colour::fromRGB(15, 17, 20), bounds.getTopLeft(),
                            juce::Colour::fromRGB(32, 34, 39), bounds.getBottomRight(), false);
    bg.addColour(0.4, juce::Colour::fromRGB(18, 20, 24));
    g.setGradientFill(bg);
    g.fillAll();

    auto topStrip = bounds.removeFromTop(74.0f);
    juce::ColourGradient strip(juce::Colour::fromRGB(12, 14, 18), topStrip.getTopLeft(),
                               juce::Colour::fromRGB(30, 33, 38), topStrip.getBottomRight(), false);
    g.setGradientFill(strip);
    g.fillRect(topStrip);

    g.setColour(juce::Colours::white.withAlpha(0.09f));
    g.drawLine(0.0f, topStrip.getBottom(), (float) getWidth(), topStrip.getBottom(), 1.0f);

    auto mainPanel = bounds.reduced(14.0f, 10.0f);
    g.setColour(theme.panelBg.withAlpha(0.96f));
    g.fillRoundedRectangle(mainPanel, 22.0f);

    g.setColour(theme.panelEdge.withAlpha(0.9f));
    g.drawRoundedRectangle(mainPanel, 22.0f, 3.0f);
    g.setColour(theme.accent.withAlpha(0.28f));
    g.drawRoundedRectangle(mainPanel.reduced(11.0f), 16.0f, 1.8f);

    g.setColour(theme.accent.withAlpha(0.18f));
    g.fillRoundedRectangle(mainPanel.reduced(10.0f).removeFromTop(102.0f), 12.0f);

    g.setColour(juce::Colours::black.withAlpha(0.03f));
    for (int x = 0; x < getWidth(); x += 8)
        g.drawVerticalLine(x, 78.0f, (float) getHeight() - 40.0f);

}

void AnalogueHellAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(74);

    auto panel = area.reduced(14, 10);
    auto top = panel.reduced(18, 14).removeFromTop(108);

    auto titleArea = top.removeFromLeft((int) std::round((float) panel.getWidth() * 0.42f));
    pluginTitle.setBounds(titleArea.removeFromTop(52));
    pluginSubtitle.setBounds(titleArea.removeFromTop(24));

    auto nav = top;
    nav.removeFromTop(27);
    auto navRow = nav.removeFromTop(40);
    const int presetW = 320;
    const int navGroupW = 34 + 8 + presetW + 8 + 34;
    auto centeredNav = navRow.withSizeKeepingCentre(navGroupW, 40);
    prevPreset.setBounds(centeredNav.removeFromLeft(34));
    centeredNav.removeFromLeft(8);
    presetBox.setBounds(centeredNav.removeFromLeft(presetW));
    centeredNav.removeFromLeft(8);
    nextPreset.setBounds(centeredNav.removeFromLeft(34));

    auto body = panel.reduced(18, 14);
    body.removeFromTop(112);

    sectionMacro.setBounds(body.removeFromTop(30));
    auto macro = body.removeFromTop(182);
    const int macroW = macro.getWidth() / 4;

    for (int i = 0; i < 4; ++i)
    {
        auto cell = juce::Rectangle<int>(macro.getX() + i * macroW, macro.getY(), macroW, macro.getHeight()).reduced(10, 0);
        labels[(size_t) i]->setBounds(cell.removeFromTop(22));
        sliders[(size_t) i]->setBounds(cell);
    }

    body.removeFromTop(6);
    sectionGlobal.setBounds(body.removeFromTop(30));
    auto global = body.removeFromTop(182);
    const int globalW = global.getWidth() / 4;

    for (int i = 0; i < 4; ++i)
    {
        auto cell = juce::Rectangle<int>(global.getX() + i * globalW, global.getY(), globalW, global.getHeight()).reduced(10, 0);
        labels[(size_t) (i + 4)]->setBounds(cell.removeFromTop(22));
        sliders[(size_t) (i + 4)]->setBounds(cell);
    }

}

void AnalogueHellAudioProcessorEditor::addSlider(juce::Slider& slider,
                                                 juce::Label& label,
                                                 const char* paramID,
                                                 const char* title)
{
    const auto theme = getTheme(processorRef.getModelId());

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 0.75f,
                               juce::MathConstants<float>::pi * 2.25f,
                               true);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 24);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, theme.panelBg.brighter(0.15f));
    slider.setColour(juce::Slider::textBoxTextColourId, theme.textBright);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxHighlightColourId, theme.accent.withAlpha(0.3f));
    slider.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(slider);

    label.setText(title, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(13.0f, juce::Font::plain));
    label.setColour(juce::Label::textColourId, theme.textBright);
    addAndMakeVisible(label);

    attachments.push_back(std::make_unique<Attach>(processorRef.getAPVTS(), paramID, slider));
}

void AnalogueHellAudioProcessorEditor::buildPresetMenu()
{
    presets.clear();

    auto add = [this](const juce::String& name,
                      float vDrive, float vMix, float vDepth, float vRate,
                      float vCharacter, float vNoise, float vUnstable, float vStereo)
    {
        presets.push_back({ name, vDrive, vMix, vDepth, vRate, vCharacter, vNoise, vUnstable, vStereo });
    };

    switch (processorRef.getModelId())
    {
        case 1: // Tape System
            add("Miscalibrated Reel", 0.48f, 0.64f, 0.22f, 0.40f, 0.18f, 0.08f, 0.22f, 0.36f);
            add("Worn Leader", 0.62f, 0.78f, 0.48f, 0.95f, 0.34f, 0.16f, 0.42f, 0.58f);
            add("Ghost Print", 0.38f, 0.70f, 0.82f, 0.30f, 0.46f, 0.22f, 0.56f, 0.52f);
            add("Scrape Flutter", 0.72f, 0.86f, 0.94f, 6.80f, 0.58f, 0.30f, 0.90f, 0.70f);
            add("Splice Panic", 0.78f, 0.92f, 0.72f, 4.20f, 0.66f, 0.44f, 1.00f, 0.76f);
            add("Slow Oxide Melt", 0.90f, 0.74f, 0.18f, 0.12f, 0.86f, 0.10f, 0.18f, 0.42f);
            add("Azimuth Drift", 0.56f, 0.80f, 0.64f, 2.60f, 0.40f, 0.18f, 0.52f, 1.00f);
            add("Underbiased Hit", 0.98f, 0.84f, 0.40f, 1.40f, 0.06f, 0.08f, 0.26f, 0.34f);
            add("Overbiased Fog", 0.30f, 0.88f, 0.70f, 0.24f, 1.00f, 0.14f, 0.36f, 0.40f);
            add("Haunted Master Bus", 0.66f, 0.98f, 0.90f, 2.10f, 0.62f, 0.34f, 0.86f, 0.80f);
            break;
        case 2: // BBD Grime
            add("Clock Leak", 0.46f, 1.00f, 0.78f, 5.20f, 0.52f, 0.36f, 0.70f, 0.55f);
            add("Compander Cough", 0.63f, 0.92f, 0.66f, 2.40f, 0.44f, 0.48f, 0.74f, 0.51f);
            add("Short Delay Dirt", 0.59f, 0.89f, 0.86f, 7.80f, 0.61f, 0.44f, 0.69f, 0.59f);
            add("SampleHold Teeth", 0.74f, 0.85f, 0.73f, 6.30f, 0.67f, 0.27f, 0.56f, 0.47f);
            add("Bucket Collapse", 0.81f, 0.79f, 0.58f, 3.10f, 0.72f, 0.35f, 0.83f, 0.62f);
            add("Bleed Chorus", 0.41f, 0.94f, 0.49f, 1.60f, 0.46f, 0.22f, 0.41f, 0.77f);
            add("Carbon Delay Gone", 0.70f, 0.82f, 0.68f, 4.40f, 0.64f, 0.39f, 0.75f, 0.58f);
            add("Gated Hiss Pump", 0.52f, 0.91f, 0.54f, 2.10f, 0.39f, 0.57f, 0.62f, 0.50f);
            add("Nervous Repeats", 0.56f, 0.88f, 0.81f, 8.60f, 0.53f, 0.32f, 0.89f, 0.71f);
            add("Corroded Echo Bus", 0.66f, 0.83f, 0.62f, 3.70f, 0.58f, 0.41f, 0.68f, 0.60f);
            break;
        case 3: // Core Bend
            add("Laminated Bend", 0.51f, 1.00f, 0.48f, 1.20f, 0.53f, 0.12f, 0.38f, 0.46f);
            add("Bass Pull", 0.69f, 0.90f, 0.42f, 0.44f, 0.61f, 0.09f, 0.31f, 0.50f);
            add("Flux Memory", 0.58f, 0.92f, 0.57f, 0.82f, 0.46f, 0.13f, 0.52f, 0.64f);
            add("Core Thud", 0.80f, 0.81f, 0.36f, 0.31f, 0.72f, 0.08f, 0.24f, 0.44f);
            add("DC Sneer", 0.62f, 0.87f, 0.50f, 1.80f, 0.68f, 0.16f, 0.66f, 0.55f);
            add("Ungrounded Input", 0.55f, 0.88f, 0.66f, 2.70f, 0.59f, 0.21f, 0.73f, 0.73f);
            add("Rail Push", 0.86f, 0.74f, 0.34f, 0.28f, 0.77f, 0.07f, 0.18f, 0.43f);
            add("Transformer Bloom", 0.47f, 0.96f, 0.61f, 0.90f, 0.42f, 0.11f, 0.47f, 0.62f);
            add("Broken DI", 0.73f, 0.82f, 0.43f, 1.55f, 0.69f, 0.14f, 0.57f, 0.48f);
            add("Console Iron", 0.60f, 0.89f, 0.52f, 0.70f, 0.56f, 0.10f, 0.44f, 0.58f);
            break;
        case 4: // Tube Misuse
            add("Blocking Choke", 0.77f, 0.86f, 0.68f, 0.56f, 0.64f, 0.12f, 0.59f, 0.48f);
            add("Sagging Rails", 0.66f, 0.88f, 0.47f, 0.32f, 0.53f, 0.10f, 0.36f, 0.43f);
            add("Grid Leak", 0.84f, 0.79f, 0.39f, 0.91f, 0.72f, 0.14f, 0.49f, 0.57f);
            add("Microphonic Bell", 0.49f, 0.93f, 0.81f, 2.50f, 0.45f, 0.15f, 0.76f, 0.67f);
            add("Tired Power", 0.58f, 0.91f, 0.54f, 0.48f, 0.51f, 0.11f, 0.41f, 0.45f);
            add("Cranked Recovery", 0.91f, 0.71f, 0.63f, 0.73f, 0.79f, 0.18f, 0.82f, 0.50f);
            add("Valve Collapse", 0.73f, 0.82f, 0.74f, 1.20f, 0.69f, 0.20f, 0.71f, 0.61f);
            add("Amp Room Ring", 0.55f, 0.90f, 0.88f, 3.60f, 0.47f, 0.22f, 0.84f, 0.72f);
            add("Starved Plate", 0.64f, 0.84f, 0.51f, 1.70f, 0.61f, 0.16f, 0.62f, 0.53f);
            add("Broadcast Valve", 0.52f, 0.95f, 0.42f, 0.65f, 0.43f, 0.09f, 0.33f, 0.57f);
            break;
        case 5: // Fault Fuzz
            add("Leaky Germanium", 0.72f, 0.88f, 0.58f, 1.30f, 0.39f, 0.18f, 0.71f, 0.52f);
            add("Bias Wander", 0.66f, 0.90f, 0.73f, 0.42f, 0.45f, 0.14f, 0.83f, 0.58f);
            add("Broken PushPull", 0.83f, 0.80f, 0.49f, 0.84f, 0.71f, 0.09f, 0.40f, 0.44f);
            add("Rectifier Scream", 0.88f, 0.76f, 0.81f, 1.90f, 0.62f, 0.11f, 0.52f, 0.61f);
            add("Octave Wire", 0.79f, 0.82f, 0.86f, 2.40f, 0.57f, 0.13f, 0.64f, 0.67f);
            add("Transistor Rash", 0.70f, 0.86f, 0.54f, 0.74f, 0.67f, 0.10f, 0.38f, 0.47f);
            add("Thermal Drift", 0.62f, 0.91f, 0.42f, 0.18f, 0.41f, 0.08f, 0.88f, 0.50f);
            add("Ripped Speaker", 0.94f, 0.68f, 0.70f, 0.96f, 0.83f, 0.15f, 0.59f, 0.56f);
            add("Shattered Cassette", 0.76f, 0.79f, 0.66f, 1.55f, 0.74f, 0.20f, 0.77f, 0.64f);
            add("Knife Edge Mono", 0.81f, 0.84f, 0.37f, 0.63f, 0.69f, 0.07f, 0.33f, 0.12f);
            break;
        case 6: // Bus Overload
            add("Overdriven Summing", 0.58f, 0.66f, 0.28f, 0.48f, 0.44f, 0.06f, 0.18f, 0.34f);
            add("Slew Flatten", 0.92f, 0.86f, 0.72f, 0.26f, 0.18f, 0.10f, 0.22f, 0.28f);
            add("Interstage Crunch", 0.78f, 0.80f, 0.76f, 1.70f, 0.62f, 0.18f, 0.51f, 0.54f);
            add("Bleed Bus", 0.42f, 0.64f, 0.44f, 0.40f, 0.70f, 0.08f, 0.20f, 0.98f);
            add("Redline Glue", 0.84f, 0.72f, 0.56f, 0.20f, 0.36f, 0.06f, 0.12f, 0.46f);
            add("Ghost Crosstalk", 0.52f, 0.74f, 0.82f, 1.20f, 0.58f, 0.20f, 0.62f, 1.00f);
            add("Desk Meltdown", 0.98f, 0.90f, 0.92f, 3.40f, 0.24f, 0.32f, 0.94f, 0.70f);
            add("Saturated Sends", 0.74f, 0.78f, 0.66f, 1.10f, 0.56f, 0.14f, 0.40f, 0.50f);
            add("Wide Bus Film", 0.48f, 0.82f, 0.38f, 0.72f, 0.64f, 0.07f, 0.18f, 1.00f);
            add("Master Push", 0.88f, 0.84f, 0.64f, 0.92f, 0.40f, 0.09f, 0.34f, 0.52f);
            break;
        case 7: // Unstable Filter
            add("Sub Melt LP", 0.26f, 0.62f, 0.32f, 0.40f, 0.08f, 0.09f, 0.22f, 0.42f);
            add("Mid Bark BP", 0.68f, 0.78f, 0.74f, 1.80f, 0.32f, 0.26f, 0.54f, 0.50f);
            add("Needle HP", 0.82f, 0.86f, 0.58f, 2.90f, 0.58f, 0.72f, 0.72f, 0.46f);
            add("Near SelfOsc", 0.94f, 0.90f, 0.92f, 0.95f, 0.84f, 0.18f, 0.88f, 0.56f);
            add("Thermal Sweep", 0.52f, 0.84f, 0.76f, 6.10f, 0.46f, 0.34f, 0.96f, 0.68f);
            add("Instance Mismatch", 0.62f, 0.72f, 0.64f, 2.20f, 0.24f, 0.42f, 0.66f, 1.00f);
            add("Filter Panic", 0.98f, 0.98f, 1.00f, 8.40f, 0.92f, 0.95f, 1.00f, 0.70f);
            add("Dusty Mod Board", 0.44f, 0.66f, 0.42f, 1.10f, 0.20f, 0.20f, 0.36f, 0.74f);
            add("Chaotic VCF", 0.88f, 0.82f, 0.86f, 4.80f, 0.70f, 0.58f, 0.94f, 0.80f);
            add("Liquid Ring Bus", 0.58f, 0.76f, 0.66f, 3.30f, 0.48f, 0.30f, 0.62f, 0.62f);
            break;
        case 8: // Spring Chaos
            add("Boing Chamber", 0.42f, 0.62f, 0.36f, 0.55f, 0.44f, 0.10f, 0.28f, 0.50f);
            add("Crash Rail", 0.86f, 0.90f, 0.92f, 5.40f, 0.77f, 0.40f, 0.96f, 0.64f);
            add("Hum Pickup", 0.30f, 0.48f, 0.22f, 0.22f, 0.40f, 0.70f, 0.14f, 0.42f);
            add("Bent Tank", 0.69f, 0.80f, 0.67f, 1.60f, 0.66f, 0.24f, 0.58f, 0.72f);
            add("Feedback Alley", 0.88f, 0.97f, 0.98f, 7.10f, 0.86f, 0.30f, 0.98f, 0.76f);
            add("Mechanical Ghost", 0.54f, 0.73f, 0.51f, 2.80f, 0.58f, 0.46f, 0.42f, 0.83f);
            add("Plate Gone Wrong", 0.72f, 0.86f, 0.76f, 3.20f, 0.62f, 0.19f, 0.66f, 0.58f);
            add("Broken Return", 0.91f, 0.58f, 0.47f, 1.10f, 0.74f, 0.52f, 0.62f, 0.36f);
            add("Ambient Coil", 0.38f, 0.88f, 0.84f, 0.80f, 0.51f, 0.18f, 0.34f, 0.92f);
            add("Howl Network", 0.96f, 1.00f, 1.00f, 9.60f, 0.92f, 0.62f, 1.00f, 0.84f);
            break;
        case 9: // Vinyl Weapon
            add("OffCenter Drift", 0.42f, 0.62f, 0.70f, 0.30f, 0.40f, 0.12f, 0.24f, 0.52f);
            add("Inner Groove Burn", 0.84f, 0.86f, 0.52f, 1.70f, 0.96f, 0.14f, 0.34f, 0.30f);
            add("Static Storm", 0.56f, 0.92f, 0.76f, 3.80f, 0.58f, 0.74f, 0.98f, 0.64f);
            add("Warped Pressing", 0.64f, 0.88f, 0.98f, 0.18f, 0.62f, 0.16f, 0.76f, 0.82f);
            add("Needle Dust", 0.38f, 0.54f, 0.28f, 2.20f, 0.36f, 0.54f, 0.52f, 0.46f);
            add("Bearing Grind", 0.90f, 0.78f, 0.46f, 0.92f, 0.78f, 0.36f, 0.48f, 0.52f);
            add("Disc Erosion", 0.76f, 0.82f, 0.60f, 1.30f, 0.86f, 0.24f, 0.70f, 0.60f);
            add("Haunted Turntable", 0.70f, 0.90f, 0.88f, 2.80f, 0.74f, 0.46f, 1.00f, 0.80f);
            add("Mono Cutter", 0.80f, 0.84f, 0.34f, 0.62f, 0.90f, 0.10f, 0.26f, 0.02f);
            add("Late Night Archive", 0.30f, 0.48f, 0.22f, 0.44f, 0.28f, 0.08f, 0.18f, 0.44f);
            break;
        case 10: // CalLab Ghost
            add("Leaky Sine Rig", 0.49f, 0.96f, 0.54f, 1.90f, 0.42f, 0.29f, 0.55f, 0.62f);
            add("Probe Loading", 0.58f, 0.90f, 0.41f, 0.84f, 0.61f, 0.14f, 0.37f, 0.48f);
            add("FM Sideband Mist", 0.63f, 0.86f, 0.72f, 3.30f, 0.57f, 0.21f, 0.68f, 0.70f);
            add("AM Broadcast Bleed", 0.46f, 0.94f, 0.66f, 1.30f, 0.51f, 0.40f, 0.59f, 0.56f);
            add("Nearby Station", 0.54f, 0.89f, 0.79f, 2.70f, 0.47f, 0.35f, 0.82f, 0.73f);
            add("VCO Drift Bed", 0.67f, 0.82f, 0.58f, 0.26f, 0.64f, 0.18f, 0.90f, 0.65f);
            add("Image Frequency", 0.72f, 0.80f, 0.47f, 4.40f, 0.69f, 0.31f, 0.74f, 0.57f);
            add("Scope Lab Panic", 0.80f, 0.74f, 0.86f, 5.20f, 0.76f, 0.43f, 0.95f, 0.69f);
            add("Triangle Bleed", 0.43f, 0.97f, 0.35f, 1.10f, 0.38f, 0.17f, 0.44f, 0.54f);
            add("Calibration Ruin", 0.61f, 0.85f, 0.62f, 2.20f, 0.58f, 0.25f, 0.79f, 0.61f);
            break;
        default:
            add("Default", 0.50f, 1.00f, 0.50f, 1.50f, 0.50f, 0.25f, 0.50f, 0.50f);
            break;
    }

    presetChangeInternal = true;
    presetBox.clear(juce::dontSendNotification);
    for (int i = 0; i < (int) presets.size(); ++i)
        presetBox.addItem(presets[(size_t) i].name, i + 1);
    presetChangeInternal = false;
}

void AnalogueHellAudioProcessorEditor::applyPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= (int) presets.size())
        return;

    const auto& p = presets[(size_t) presetIndex];
    auto& apvts = processorRef.getAPVTS();

    auto set = [&apvts](const char* id, float value)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    };

    set("drive", p.drive);
    set("mix", p.mix);
    set("depth", p.depth);
    set("rate", p.rate);
    set("character", p.character);
    set("noise", p.noise);
    set("unstable", p.unstable);
    set("stereo", p.stereo);
}

void AnalogueHellAudioProcessorEditor::selectPresetRelative(int delta)
{
    if (presets.empty())
        return;

    int index = presetBox.getSelectedItemIndex();
    if (index < 0)
        index = 0;

    index += delta;
    if (index < 0)
        index = (int) presets.size() - 1;
    else if (index >= (int) presets.size())
        index = 0;

    presetChangeInternal = true;
    presetBox.setSelectedItemIndex(index, juce::sendNotificationSync);
    presetChangeInternal = false;
    applyPreset(index);
}
