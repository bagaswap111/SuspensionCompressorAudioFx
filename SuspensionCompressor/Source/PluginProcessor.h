#pragma once

#include <JuceHeader.h>
#include "SuspensionModel.h"
#include "SuspensionPresets.h"

class SuspensionCompressorAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SuspensionCompressorAudioProcessor();
    ~SuspensionCompressorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameters
    struct Parameters
    {
        juce::AudioParameterFloat* threshold = nullptr;
        juce::AudioParameterFloat* ratio = nullptr;
        juce::AudioParameterFloat* attack = nullptr;
        juce::AudioParameterFloat* release = nullptr;
        juce::AudioParameterFloat* knee = nullptr;
        juce::AudioParameterFloat* makeupGain = nullptr;
        juce::AudioParameterFloat* mix = nullptr;
        juce::AudioParameterFloat* damping = nullptr; // ζ (zeta)
        juce::AudioParameterChoice* suspensionType = nullptr;
        juce::AudioParameterBool* autoRelease = nullptr;
        juce::AudioParameterFloat* lookAhead = nullptr;
    };

    Parameters params;
    SuspensionModel suspension;
    
    // State
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    float currentGainReduction = 0.0f;
    float leftChannelState = 0.0f;
    float rightChannelState = 0.0f;
    float leftVelocity = 0.0f;
    float rightVelocity = 0.0f;
    
    // Look-ahead buffer
    juce::AudioBuffer<float> lookAheadBuffer;
    int lookAheadSamples = 0;
    int writePos = 0;
    int readPos = 0;
    
    // Metering
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    
    std::atomic<bool> parametersChanged {false};

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuspensionCompressorAudioProcessor)
};
