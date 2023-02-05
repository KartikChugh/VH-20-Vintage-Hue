/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class KartikCPFinalVH20AudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Slider::Listener, juce::ComboBox::Listener
{
public:
    KartikCPFinalVH20AudioProcessorEditor (KartikCPFinalVH20AudioProcessor&);
    ~KartikCPFinalVH20AudioProcessorEditor() override;

    void addRotarySlider(juce::Slider& slider, juce::AudioProcessorParameter* processorParam, juce::Rectangle<int>& bounds);
    void addComboBox(juce::ComboBox& cb, juce::AudioProcessorParameter* processorParam, juce::Rectangle<int>& bounds, const juce::StringArray& items);
    void addLabel(juce::Label& label, const juce::String& text, juce::Component& component);

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* cb) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KartikCPFinalVH20AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KartikCPFinalVH20AudioProcessorEditor)

    // Processor parameter numbers
    constexpr static int PARAM_MIX = 0;
    constexpr static int PARAM_GRIT_AMOUNT = 1;
    constexpr static int PARAM_DUST_AMOUNT = 2;
    constexpr static int PARAM_WEAR_AMOUNT = 3;
    //constexpr static int PARAM_GRIT_TYPE = 4;
    constexpr static int PARAM_DUST_TYPE = 4;
    
    // GUI Components
    juce::Slider mMixSlider, mGritAmountSlider, mDustAmountSlider, mWearAmountSlider;
    juce::ComboBox mDustTypeSelector;
    juce::Label mGritLabel, mDustLabel, mWearLabel, mTitleLabel;
};
