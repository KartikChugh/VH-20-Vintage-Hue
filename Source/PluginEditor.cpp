/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KartikCPFinalVH20AudioProcessorEditor::KartikCPFinalVH20AudioProcessorEditor (KartikCPFinalVH20AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);

    auto& params = processor.getParameters();

    addRotarySlider(mGritAmountSlider, params.getUnchecked(PARAM_GRIT_AMOUNT), juce::Rectangle<int>(35, 120, 170, 170));
    addRotarySlider(mDustAmountSlider, params.getUnchecked(PARAM_DUST_AMOUNT), juce::Rectangle<int>(215, 120, 170, 170));
    addRotarySlider(mWearAmountSlider, params.getUnchecked(PARAM_WEAR_AMOUNT), juce::Rectangle<int>(395, 120, 170, 170));
    
    mDustAmountSlider.setSkewFactorFromMidPoint(25);

    addComboBox(mDustTypeSelector, params.getUnchecked(PARAM_DUST_TYPE), juce::Rectangle<int>(260, 180, 75, 20), ((juce::AudioParameterChoice*)params.getUnchecked(PARAM_DUST_TYPE))->choices);

    addLabel(mGritLabel, "GRIT", mGritAmountSlider);
    addLabel(mDustLabel, "FILM", mDustAmountSlider);
    addLabel(mWearLabel, "WEAR", mWearAmountSlider);

    mTitleLabel.setText("VH-20: Vintage Hue", juce::dontSendNotification);
    mTitleLabel.setJustificationType(juce::Justification::centred);
    mTitleLabel.setBounds(0, 20, 600, 50);
    addAndMakeVisible(mTitleLabel);
}

KartikCPFinalVH20AudioProcessorEditor::~KartikCPFinalVH20AudioProcessorEditor()
{
}

// Removed thumb color change
void KartikCPFinalVH20AudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
    
    int id;

    if (slider == &mMixSlider)
    {
        id = PARAM_MIX;
    }
    else if (slider == &mGritAmountSlider) {
        id = PARAM_GRIT_AMOUNT;
    }
    else if (slider == &mDustAmountSlider) {
        id = PARAM_DUST_AMOUNT;
    }
    else if (slider == &mWearAmountSlider) {
        id = PARAM_WEAR_AMOUNT;
    }
    else {
        return;
    }

    auto& params = processor.getParameters();
    juce::AudioParameterFloat* param = (juce::AudioParameterFloat*)params.getUnchecked(id);
    *param = slider->getValue();
}

void KartikCPFinalVH20AudioProcessorEditor::comboBoxChanged(juce::ComboBox* cb) {
    int id;
    if (cb == &mDustTypeSelector) {
        id = PARAM_DUST_TYPE;
    }
    else {
        return;
    }

    auto& params = processor.getParameters();
    juce::AudioParameterChoice* param = (juce::AudioParameterChoice*)params.getUnchecked(id);
    *param = cb->getSelectedItemIndex();
}

void KartikCPFinalVH20AudioProcessorEditor::addComboBox(juce::ComboBox& cb, juce::AudioProcessorParameter* processorParam, juce::Rectangle<int>& bounds, const juce::StringArray& items) {
    juce::AudioParameterChoice* param = (juce::AudioParameterChoice*)processorParam;

    cb.setBounds(bounds);
    cb.addItemList(items, 1);
    cb.setSelectedItemIndex(param->getIndex());
    cb.addListener(this);
    addAndMakeVisible(cb);
}

void KartikCPFinalVH20AudioProcessorEditor::addRotarySlider(juce::Slider& slider, juce::AudioProcessorParameter* processorParam, juce::Rectangle<int>& bounds)
{
    juce::AudioParameterFloat* param = (juce::AudioParameterFloat*)processorParam;

    slider.setBounds(bounds);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider.setDoubleClickReturnValue(true, param->convertFrom0to1(0.5)); // Revert value is midpoint
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::ghostwhite);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 10);
    slider.setRange(param->range.start, param->range.end, param->range.interval);
    slider.setValue(*param);
    slider.addListener(this);
    addAndMakeVisible(slider);
}

void KartikCPFinalVH20AudioProcessorEditor::addLabel(juce::Label& label, const juce::String& text, juce::Component& component) {
    label.setText(text, juce::dontSendNotification);
    label.attachToComponent(&component, false);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

//==============================================================================
void KartikCPFinalVH20AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colour(25.0f, 25.0f, 25.0f));
    getLookAndFeel().setDefaultSansSerifTypefaceName("Consolas");
    //g.drawText("VH-20: Vintage Hue", 100, 50, 300, 100, juce::Justification::centred, false);
}

void KartikCPFinalVH20AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
