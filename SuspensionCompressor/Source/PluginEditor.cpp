#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SuspensionPresets.h"

//==============================================================================
SuspensionCompressorAudioProcessorEditor::RotaryKnob::RotaryKnob(
    const juce::String& name, const juce::String& unit,
    float minVal, float maxVal, float defaultValue)
    : knobName(name), unitText(unit)
{
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 20);
    setRange(minVal, maxVal);
    setValue(defaultValue, juce::dontSendNotification);
    setRepaintsOnMouseActivity(true);
}

void SuspensionCompressorAudioProcessorEditor::RotaryKnob::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(4);
    auto centre = bounds.getCentre();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;

    juce::ColourGradient baseGradient(
        juce::Colour(0xff5a5a5a), centre.x - radius, centre.y - radius,
        juce::Colour(0xff3a3a3a), centre.x + radius, centre.y + radius, false);

    g.setGradientFill(baseGradient);
    g.fillEllipse(bounds.toFloat());
    g.setColour(juce::Colour(0xff6a6a6a));
    g.drawEllipse(bounds.toFloat(), 2.0f);

    const auto value = getValue();
    const auto minVal = getMinimum();
    const auto maxVal = getMaximum();
    const float normalizedValue = (value - minVal) / (maxVal - minVal);
    const float angle = -135.0f + normalizedValue * 270.0f;
    const float angleRad = juce::degreesToRadians(angle - 90.0f);

    juce::Path indicatorPath;
    const float lineLength = radius * 0.7f;
    const float innerRadius = radius * 0.3f;
    indicatorPath.startNewSubPath(
        centre.x + innerRadius * std::cos(angleRad),
        centre.y + innerRadius * std::sin(angleRad));
    indicatorPath.lineTo(
        centre.x + lineLength * std::cos(angleRad),
        centre.y + lineLength * std::sin(angleRad));

    g.setColour(isMouseOver() ? juce::Colour(0xffdc143c) : juce::Colour(0xffffaa00));
    g.strokePath(indicatorPath, juce::PathStrokeType(3.0f));

    juce::String valueText;
    if (std::abs(value) >= 100.0f)
        valueText = juce::String(static_cast<int>(value));
    else if (std::abs(value) >= 10.0f)
        valueText = juce::String(value, 1);
    else
        valueText = juce::String(value, 2);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(valueText + " " + unitText,
               bounds.removeFromBottom(20),
               juce::Justification::centred);

    g.setFont(juce::Font(9.0f));
    g.setColour(juce::Colour(0xffaaaaaa));
    g.drawText(knobName,
               bounds,
               juce::Justification::centredTop);
}

//==============================================================================
SuspensionCompressorAudioProcessorEditor::SuspensionCompressorAudioProcessorEditor(SuspensionCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::File logFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("suspension_gui_debug.txt");
    logFile.appendText("editor constructor entered\n", false, false);

    setSize (800, 500);
    centreWithSize (800, 500);
    setResizable (false, false);
    setOpaque (true);
    setVisible (true);
    setLookAndFeel(&juce::LookAndFeel_V4::getDefaultLookAndFeel());
    
    // Title
    titleLabel.setText("SUSPENSION COMPRESSOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, ferrariRed);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Suspension type combo
    for (int i = 0; i < static_cast<int>(SuspensionModel::SuspensionType::NumTypes); ++i)
    {
        suspensionTypeCombo.addItem(SuspensionModel::getSuspensionName(
            static_cast<SuspensionModel::SuspensionType>(i)), i + 1);
    }
    suspensionTypeCombo.setSelectedId(2); // Double Wishbone default
    suspensionTypeCombo.addListener(this);
    addAndMakeVisible(suspensionTypeCombo);
    
    // Create knobs
    thresholdKnob = std::make_unique<RotaryKnob>("THRESHOLD", "dB", -60.0f, 0.0f, -18.0f);
    ratioKnob = std::make_unique<RotaryKnob>("RATIO", ":1", 1.0f, 20.0f, 4.0f);
    attackKnob = std::make_unique<RotaryKnob>("ATTACK", "ms", 0.1f, 100.0f, 15.0f);
    releaseKnob = std::make_unique<RotaryKnob>("RELEASE", "ms", 10.0f, 1000.0f, 200.0f);
    kneeKnob = std::make_unique<RotaryKnob>("KNEE", "dB", 0.0f, 100.0f, 30.0f);
    makeupKnob = std::make_unique<RotaryKnob>("MAKEUP", "dB", -12.0f, 24.0f, 3.0f);
    mixKnob = std::make_unique<RotaryKnob>("MIX", "%", 0.0f, 100.0f, 100.0f);
    dampingKnob = std::make_unique<RotaryKnob>("DAMPING", "ζ", 0.1f, 2.0f, 0.7f);
    lookAheadKnob = std::make_unique<RotaryKnob>("LOOKAHEAD", "ms", 0.0f, 10.0f, 0.0f);
    
    addAndMakeVisible(*thresholdKnob);
    addAndMakeVisible(*ratioKnob);
    addAndMakeVisible(*attackKnob);
    addAndMakeVisible(*releaseKnob);
    addAndMakeVisible(*kneeKnob);
    addAndMakeVisible(*makeupKnob);
    addAndMakeVisible(*mixKnob);
    addAndMakeVisible(*dampingKnob);
    addAndMakeVisible(*lookAheadKnob);
    
    // Create attachments
    if (audioProcessor.apvts != nullptr)
    {
        thresholdAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.threshold, *thresholdKnob, nullptr);
        ratioAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.ratio, *ratioKnob, nullptr);
        attackAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.attack, *attackKnob, nullptr);
        releaseAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.release, *releaseKnob, nullptr);
        kneeAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.knee, *kneeKnob, nullptr);
        makeupAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.makeupGain, *makeupKnob, nullptr);
        mixAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.mix, *mixKnob, nullptr);
        dampingAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.damping, *dampingKnob, nullptr);
        lookAheadAttachment = std::make_unique<juce::SliderParameterAttachment>(*audioProcessor.params.lookAhead, *lookAheadKnob, nullptr);

        auto* suspensionParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts->getParameter("suspensionType"));
        if (suspensionParam != nullptr)
            suspensionTypeCombo.setSelectedId (suspensionParam->getIndex() + 1, juce::dontSendNotification);
    }
    
    // Gain reduction meter
    grMeterLabel.setText("GAIN REDUCTION", juce::dontSendNotification);
    grMeterLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    grMeterLabel.setFont(juce::Font(10.0f));
    grMeterLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grMeterLabel);
    
    grMeterProgress = 0.0;
    grMeter.setColour(juce::ProgressBar::backgroundColourId, matteAluminum);
    grMeter.setColour(juce::ProgressBar::foregroundColourId, ferrariRed);
    addAndMakeVisible(grMeter);
    
    // Car info labels
    carInfoLabel.setText("", juce::dontSendNotification);
    carInfoLabel.setColour(juce::Label::textColourId, porscheYellow);
    carInfoLabel.setFont(juce::Font(11.0f, juce::Font::italic));
    carInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(carInfoLabel);
    
    suspensionInfoLabel.setText("", juce::dontSendNotification);
    suspensionInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
    suspensionInfoLabel.setFont(juce::Font(9.0f));
    suspensionInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(suspensionInfoLabel);
    
    // Start timer for UI updates
    startTimerHz(30);
}

SuspensionCompressorAudioProcessorEditor::~SuspensionCompressorAudioProcessorEditor()
{
    stopTimer();
}

void SuspensionCompressorAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &suspensionTypeCombo)
    {
        const auto selectedIndex = suspensionTypeCombo.getSelectedId() - 1;
        if (audioProcessor.params.suspensionType != nullptr)
            *audioProcessor.params.suspensionType = selectedIndex;
    }
}

void SuspensionCompressorAudioProcessorEditor::timerCallback()
{
    // Update gain reduction meter
    float gr = audioProcessor.currentGainReduction;
    grMeterProgress = juce::jmap(gr, 0.0f, 20.0f, 0.0f, 1.0f);
    
    // Update car info based on selected suspension
    int selectedIndex = suspensionTypeCombo.getSelectedId() - 1;
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(SuspensionModel::SuspensionType::NumTypes))
    {
        auto type = static_cast<SuspensionModel::SuspensionType>(selectedIndex);
        carInfoLabel.setText(juce::String(SuspensionModel::getCarExample(type)), juce::dontSendNotification);
        suspensionInfoLabel.setText(juce::String("Mass: ") + juce::String(audioProcessor.suspension.getParams().mass, 2) + 
                                    " kg | Spring: " + juce::String(audioProcessor.suspension.getParams().springConstant, 0) +
                                    " N/m | ζ: " + juce::String(audioProcessor.suspension.getDamping(), 2),
                                    juce::dontSendNotification);
    }
}

//==============================================================================
void SuspensionCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background gradient (carbon fiber effect)
    juce::ColourGradient backgroundGradient(
        carbonBlack, 0, 0,
        juce::Colour(0xff2a2a2a), 0, getHeight(), false);
    
    g.setGradientFill(backgroundGradient);
    g.fillAll();
    
    // Top accent line (Ferrari red)
    g.setColour(ferrariRed);
    g.fillRect(0, 0, getWidth(), 3);
}

void SuspensionCompressorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Title area
    auto titleArea = bounds.removeFromTop(40);
    titleLabel.setBounds(titleArea);
    
    // Suspension selector
    auto selectorArea = bounds.removeFromTop(30);
    suspensionTypeCombo.setBounds(selectorArea.withWidth(300).withX(selectorArea.getCentreX() - 150));
    
    // Car info
    auto infoArea = bounds.removeFromTop(25);
    carInfoLabel.setBounds(infoArea.withWidth(400).withX(infoArea.getCentreX() - 200));
    suspensionInfoLabel.setBounds(infoArea.withWidth(500).withX(infoArea.getCentreX() - 250).withY(infoArea.getY() + 15));
    
    // Knob area
    auto knobArea = bounds.removeFromTop(180);
    auto knobRow = knobArea.reduced(20);
    
    // First row of knobs: Threshold, Ratio, Attack, Release, Knee
    int knobSize = 90;
    int spacing = 15;
    int totalWidth = 5 * knobSize + 4 * spacing;
    int startX = (knobRow.getWidth() - totalWidth) / 2;
    
    thresholdKnob->setBounds(startX, 0, knobSize, knobSize);
    ratioKnob->setBounds(startX + knobSize + spacing, 0, knobSize, knobSize);
    attackKnob->setBounds(startX + 2*(knobSize + spacing), 0, knobSize, knobSize);
    releaseKnob->setBounds(startX + 3*(knobSize + spacing), 0, knobSize, knobSize);
    kneeKnob->setBounds(startX + 4*(knobSize + spacing), 0, knobSize, knobSize);
    
    addAndMakeVisible(*thresholdKnob);
    addAndMakeVisible(*ratioKnob);
    addAndMakeVisible(*attackKnob);
    addAndMakeVisible(*releaseKnob);
    addAndMakeVisible(*kneeKnob);
    
    // Second row: Makeup, Mix, Damping, Look-ahead, GR Meter
    auto secondRow = knobArea.removeFromTop(knobSize + 30);
    int totalWidth2 = 4 * knobSize + 3 * spacing + 150; // 4 knobs + meter
    startX = (secondRow.getWidth() - totalWidth2) / 2;
    
    makeupKnob->setBounds(startX, 30, knobSize, knobSize);
    mixKnob->setBounds(startX + knobSize + spacing, 30, knobSize, knobSize);
    dampingKnob->setBounds(startX + 2*(knobSize + spacing), 30, knobSize, knobSize);
    lookAheadKnob->setBounds(startX + 3*(knobSize + spacing), 30, knobSize, knobSize);
    
    // Gain reduction meter
    auto meterArea = secondRow.withWidth(150).withX(secondRow.getRight() - 150).withY(20);
    grMeterLabel.setBounds(meterArea.removeFromTop(20));
    grMeter.setBounds(meterArea.reduced(10, 5));
    
    addAndMakeVisible(*makeupKnob);
    addAndMakeVisible(*mixKnob);
    addAndMakeVisible(*dampingKnob);
    addAndMakeVisible(*lookAheadKnob);
}
