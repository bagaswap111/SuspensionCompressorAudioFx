#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class SuspensionCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                  private juce::Timer,
                                                  private juce::ComboBox::Listener
{
public:
    //==============================================================================
    SuspensionCompressorAudioProcessorEditor(SuspensionCompressorAudioProcessor&);
    ~SuspensionCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    
    void timerCallback() override;

private:
    //==============================================================================
    SuspensionCompressorAudioProcessor& audioProcessor;

    // UI Components
    juce::Label titleLabel;
    juce::ComboBox suspensionTypeCombo;
    
    // Knobs
    class RotaryKnob : public juce::Slider
    {
    public:
        RotaryKnob(const juce::String& name, const juce::String& unit,
                   float minVal, float maxVal, float defaultValue);

        void paint(juce::Graphics& g) override;
        juce::String getName() const { return knobName; }
        juce::String getUnit() const { return unitText; }

    private:
        juce::String knobName;
        juce::String unitText;
    };
    
    std::unique_ptr<RotaryKnob> thresholdKnob;
    std::unique_ptr<RotaryKnob> ratioKnob;
    std::unique_ptr<RotaryKnob> attackKnob;
    std::unique_ptr<RotaryKnob> releaseKnob;
    std::unique_ptr<RotaryKnob> kneeKnob;
    std::unique_ptr<RotaryKnob> makeupKnob;
    std::unique_ptr<RotaryKnob> mixKnob;
    std::unique_ptr<RotaryKnob> dampingKnob;
    std::unique_ptr<RotaryKnob> lookAheadKnob;
    
    // Attachments
    std::unique_ptr<juce::SliderParameterAttachment> thresholdAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> ratioAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> attackAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> releaseAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> kneeAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> makeupAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> mixAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> dampingAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> suspensionAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> autoReleaseAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> lookAheadAttachment;
    
    // Metering
    juce::Label grMeterLabel;
    double grMeterProgress = 0.0;
    juce::ProgressBar grMeter { grMeterProgress };
    
    // Info panel
    juce::Label carInfoLabel;
    juce::Label suspensionInfoLabel;
    
    // Custom colors
    inline static const juce::Colour carbonBlack      = juce::Colour(0xff1a1a1a);
    inline static const juce::Colour matteAluminum    = juce::Colour(0xff2d2d2d);
    inline static const juce::Colour brushedSteel     = juce::Colour(0xff4a4a4a);
    inline static const juce::Colour ferrariRed       = juce::Colour(0xffdc143c);
    inline static const juce::Colour lamboOrange      = juce::Colour(0xffff6600);
    inline static const juce::Colour porscheYellow    = juce::Colour(0xffffd700);
    inline static const juce::Colour bugattiBlue      = juce::Colour(0xff0066cc);
    inline static const juce::Colour astonGreen       = juce::Colour(0xff00cc66);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuspensionCompressorAudioProcessorEditor)
};
