/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "StkLite-4.6.2/DelayL.h"
#include "StkLite-4.6.2/Fir.h"
#include "StkLite-4.6.2/BiQuad.h"
#include "StkLite-4.6.2/PoleZero.h"
#include "Mu45LFO/Mu45LFO.h"

//==============================================================================
/**
*/
class KartikCPFinalVH20AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    KartikCPFinalVH20AudioProcessor();
    ~KartikCPFinalVH20AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KartikCPFinalVH20AudioProcessor)

    void calcAlgorithmParams();
    int calcMsecToSamps(float msec);
    float distort(float samp);

    // User Parameters
    juce::AudioParameterFloat* mMixParam;

    juce::AudioParameterFloat* mGritAmountParam;
    juce::AudioParameterFloat* mDustAmountParam;
    juce::AudioParameterFloat* mWearAmountParam;

    juce::AudioParameterChoice* mDustTypeParam;

    // Algorithm Parameters
    float mFs;
    juce::NormalisableRange<float> wobbleDelayRange;

    float noiseAmpl;
    float noiseAmplDoubled;

    float distortStrength;
    float distortDrive;
    float distortDriveGainCompensation;
    float delayMin;

    // Constant Parameters
    constexpr static float WOBBLE_DELAY_FLOOR_MIN = 0;
    constexpr static float WOBBLE_DELAY_FLOOR_MAX = 20;
    constexpr static float WOBBLE_DELAY_DEPTH = 3;
    constexpr static float LFO_SLOW = 0.4;

    // Signal Processors
    juce::Random noiseGen;
    Mu45LFO mLfo;
    stk::DelayL mWobbleDelayLeft, mWobbleDelayRight;
    stk::Fir mAntiAliasLeft, mAntialiasRight;
    stk::BiQuad mPinkLpfLeft, mPinkLpfRight;
    stk::BiQuad mInterpolLpfLeft, mInterpolLpfRight;
};
