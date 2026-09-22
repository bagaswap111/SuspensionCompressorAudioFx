#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SuspensionCompressorAudioProcessor::SuspensionCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Initialize parameters with APVTS
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Threshold: -60 to 0 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "threshold", "Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -18.0f));
    
    // Ratio: 1:1 to 20:1
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "ratio", "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f),
        4.0f));
    
    // Attack: 0.1 to 100 ms
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f),
        15.0f));
    
    // Release: 10 to 1000 ms
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f),
        200.0f));
    
    // Knee: 0 to 100 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "knee", "Knee",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f));
    
    // Makeup Gain: -12 to +24 dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "makeupGain", "Makeup Gain",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        3.0f));
    
    // Mix: 0 to 100%
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f));
    
    // Damping (ζ): 0.1 to 2.0
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "damping", "Damping",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f),
        0.7f));
    
    // Suspension Type
    juce::StringArray suspensionTypes;
    for (int i = 0; i < static_cast<int>(SuspensionModel::SuspensionType::NumTypes); ++i)
    {
        suspensionTypes.add(SuspensionModel::getSuspensionName(
            static_cast<SuspensionModel::SuspensionType>(i)));
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "suspensionType", "Suspension Type",
        suspensionTypes, 1)); // Default to Double Wishbone
    
    // Auto Release
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "autoRelease", "Auto Release", false));
    
    // Look-ahead: 0 to 10 ms
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lookAhead", "Look-ahead",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        0.0f));
    
    apvts = std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, "SuspensionCompressor", std::move(layout));
    
    // Set parameter pointers
    params.threshold = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("threshold"));
    params.ratio = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("ratio"));
    params.attack = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("attack"));
    params.release = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("release"));
    params.knee = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("knee"));
    params.makeupGain = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("makeupGain"));
    params.mix = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("mix"));
    params.damping = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("damping"));
    params.suspensionType = dynamic_cast<juce::AudioParameterChoice*>(apvts->getParameter("suspensionType"));
    params.autoRelease = dynamic_cast<juce::AudioParameterBool*>(apvts->getParameter("autoRelease"));
    params.lookAhead = dynamic_cast<juce::AudioParameterFloat*>(apvts->getParameter("lookAhead"));
}

SuspensionCompressorAudioProcessor::~SuspensionCompressorAudioProcessor()
{
}

//==============================================================================
void SuspensionCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    suspension.prepare(sampleRate);
    
    // Calculate look-ahead samples
    float lookAheadMs = params.lookAhead->get();
    lookAheadSamples = static_cast<int>(lookAheadMs * sampleRate / 1000.0f);
    
    // Resize look-ahead buffer
    lookAheadBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock + lookAheadSamples + 100);
    lookAheadBuffer.clear();
    
    writePos = 0;
    readPos = lookAheadSamples;
    
    leftChannelState = 0.0f;
    rightChannelState = 0.0f;
    leftVelocity = 0.0f;
    rightVelocity = 0.0f;
    currentGainReduction = 0.0f;
}

void SuspensionCompressorAudioProcessor::releaseResources()
{
    lookAheadBuffer.setSize(1, 1);
    lookAheadBuffer.clear();
}

bool SuspensionCompressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
    #else
    // Support mono and stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Check input matches output
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
    #endif
}

void SuspensionCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Get parameter values
    float threshold = params.threshold->get();
    float ratio = params.ratio->get();
    float knee = params.knee->get();
    float makeupGainDb = params.makeupGain->get();
    float mixPercent = params.mix->get() / 100.0f;
    float dampingZeta = params.damping->get();
    int suspensionTypeIndex = params.suspensionType->getIndex();
    float lookAheadMs = params.lookAhead->get();
    
    // Update suspension model
    auto newSuspensionType = static_cast<SuspensionModel::SuspensionType>(suspensionTypeIndex);
    if (newSuspensionType != suspension.getSuspensionType())
    {
        suspension.setSuspensionType(newSuspensionType);
    }
    
    suspension.setDamping(dampingZeta);
    
    // Update look-ahead
    int newLookAheadSamples = static_cast<int>(lookAheadMs * getSampleRate() / 1000.0f);
    if (newLookAheadSamples != lookAheadSamples)
    {
        lookAheadSamples = newLookAheadSamples;
        readPos = writePos - lookAheadSamples;
        if (readPos < 0)
            readPos += lookAheadBuffer.getNumSamples();
    }
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float inputSample = channelData[sample];
            
            // Convert to dB (with floor to avoid log(0))
            float inputDb = 20.0f * juce::Decibels::gainToDecibels(juce::jmax(0.0001f, std::abs(inputSample)));
            
            // Apply soft knee
            float effectiveThreshold = threshold;
            float effectiveRatio = ratio;
            
            if (knee > 0.0f && inputDb > threshold - knee && inputDb < threshold + knee)
            {
                // In the knee region
                float kneeProgress = (inputDb - (threshold - knee)) / (2.0f * knee);
                effectiveRatio = 1.0f + (ratio - 1.0f) * kneeProgress;
            }
            
            // Process through suspension model
            float gainReduction = suspension.processSample(inputDb, threshold, effectiveRatio);
            
            // Convert gain reduction from dB to linear
            float gainLinear = juce::Decibels::decibelsToGain(-gainReduction);
            
            // Apply makeup gain
            float makeupLinear = juce::Decibels::decibelsToGain(makeupGainDb);
            
            // Mix dry/wet
            float processedSample = inputSample * gainLinear * makeupLinear;
            channelData[sample] = inputSample * (1.0f - mixPercent) + processedSample * mixPercent;
            
            // Update metering (peak)
            float absVal = std::abs(channelData[sample]);
            if (absVal > inputLevel)
                inputLevel = absVal;
            if (absVal > outputLevel)
                outputLevel = absVal;
        }
    }
    
    // Update current gain reduction for UI
    currentGainReduction = suspension.getCurrentGainReduction();
    
    // Decay metering values
    inputLevel *= 0.999f;
    outputLevel *= 0.999f;
}

//==============================================================================
juce::AudioProcessorEditor* SuspensionCompressorAudioProcessor::createEditor()
{
    return new SuspensionCompressorAudioProcessorEditor(*this);
}

//==============================================================================
void SuspensionCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts->copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SuspensionCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(apvts->state.getType()))
        {
            apvts->replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SuspensionCompressorAudioProcessor();
}
