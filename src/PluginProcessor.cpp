#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogueHellAudioProcessor::AnalogueHellAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMS", createLayout())
{
}

void AnalogueHellAudioProcessor::prepareToPlay(double sampleRate, int)
{
    model.prepare(sampleRate, getTotalNumOutputChannels());
}

void AnalogueHellAudioProcessor::releaseResources()
{
}

bool AnalogueHellAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void AnalogueHellAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    model.process(buffer, getParams(), getModelId());
}

juce::AudioProcessorEditor* AnalogueHellAudioProcessor::createEditor()
{
    return new AnalogueHellAudioProcessorEditor(*this);
}

void AnalogueHellAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AnalogueHellAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AnalogueHellAudioProcessor::createLayout()
{
    using Param = juce::AudioParameterFloat;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<Param>("drive", "Drive", 0.0f, 1.0f, 0.5f));
    p.push_back(std::make_unique<Param>("mix", "Mix", 0.0f, 1.0f, 1.0f));
    p.push_back(std::make_unique<Param>("depth", "Depth", 0.0f, 1.0f, 0.5f));
    p.push_back(std::make_unique<Param>("rate", "Rate", juce::NormalisableRange<float>(0.01f, 10.0f, 0.001f, 0.35f), 1.5f));
    p.push_back(std::make_unique<Param>("character", "Character", 0.0f, 1.0f, 0.5f));
    p.push_back(std::make_unique<Param>("noise", "Noise", 0.0f, 1.0f, 0.25f));
    p.push_back(std::make_unique<Param>("unstable", "Unstable", 0.0f, 1.0f, 0.45f));
    p.push_back(std::make_unique<Param>("stereo", "Stereo", 0.0f, 1.0f, 0.5f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("reactive", "Reactive", false));

    return { p.begin(), p.end() };
}

ah::Params AnalogueHellAudioProcessor::getParams() const
{
    auto value = [this](const char* id) { return parameters.getRawParameterValue(id)->load(); };

    ah::Params p;
    p.drive = value("drive");
    p.mix = value("mix");
    p.depth = value("depth");
    p.rateHz = value("rate");
    p.character = value("character");
    p.noise = value("noise");
    p.unstable = value("unstable");
    p.stereo = value("stereo");
    p.reactive = value("reactive");
    return p;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogueHellAudioProcessor();
}
