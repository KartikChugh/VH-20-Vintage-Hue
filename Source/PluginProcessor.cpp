/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Mu45FilterCalc/Mu45FilterCalc.h"

//==============================================================================
KartikCPFinalVH20AudioProcessor::KartikCPFinalVH20AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(mMixParam = new juce::AudioParameterFloat("mix", // parameterID,
        "Mix (%)", // parameterName,
        juce::NormalisableRange<float>(0.0f, // minValue,
            100.0f, // maxValue,
            1.0f), // intervalValue
        100.0f)); // defaultValue

    addParameter(mGritAmountParam = new juce::AudioParameterFloat("gritAmount", // parameterID,
        "Grit Amount", // parameterName,
        juce::NormalisableRange<float>(0.0f, // minValue,
            100.0f, // maxValue,
            1.0f), // intervalValue
        0.0f)); // defaultValue

    addParameter(mDustAmountParam = new juce::AudioParameterFloat("dustAmount", // parameterID,
        "Dust Amount", // parameterName,
        juce::NormalisableRange<float>(0.0f, // minValue,
            100.0f, // maxValue,
            1.0f), // intervalValue
        0.0f)); // defaultValue

    addParameter(mWearAmountParam = new juce::AudioParameterFloat("wearAmount", // parameterID,
        "Wear Amount", // parameterName,
        juce::NormalisableRange<float>(0.0f, // minValue,
            100.0f, // maxValue,
            1.0f), // intervalValue
        0.0f)); // defaultValue

    addParameter(mDustTypeParam = new juce::AudioParameterChoice("dustType", // parameterID,
        "Dust Type", // parameterName,
        {"Dust", "Mist"}, // choices
        0
    ));
}

KartikCPFinalVH20AudioProcessor::~KartikCPFinalVH20AudioProcessor()
{
}

//==============================================================================
const juce::String KartikCPFinalVH20AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KartikCPFinalVH20AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KartikCPFinalVH20AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KartikCPFinalVH20AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KartikCPFinalVH20AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KartikCPFinalVH20AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KartikCPFinalVH20AudioProcessor::getCurrentProgram()
{
    return 0;
}

void KartikCPFinalVH20AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KartikCPFinalVH20AudioProcessor::getProgramName (int index)
{
    return {};
}

void KartikCPFinalVH20AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KartikCPFinalVH20AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mFs = sampleRate;

    // Interpolation & anti-image LPF: 19 kHz cutoff @ 88.2k
    mInterpolLpfLeft.setCoefficients(0.2320128165597399, 0.4640256331194798, 0.2320128165597399, -0.25503894934464993, 0.1830902155836096);
    mInterpolLpfLeft.setSampleRate(mFs * 2);

    mInterpolLpfRight.setCoefficients(0.2320128165597399, 0.4640256331194798, 0.2320128165597399, -0.25503894934464993, 0.1830902155836096);
    mInterpolLpfRight.setSampleRate(mFs * 2);

    // Pink Filter: -3db/octave
    mPinkLpfLeft.setCoefficients(0.04957526213389, -0.06305581334498, 0.01483220320740, -1.80116083982126, 0.80257737639225);
    mPinkLpfLeft.setSampleRate(mFs * 2);

    mPinkLpfRight.setCoefficients(0.04957526213389, -0.06305581334498, 0.01483220320740, -1.80116083982126, 0.80257737639225);
    mPinkLpfRight.setSampleRate(mFs * 2);

    // Tape wobble delay
    wobbleDelayRange = juce::NormalisableRange<float>(0, 0);
    mLfo.setFreq(LFO_SLOW, mFs);
    mWobbleDelayLeft.setMaximumDelay(calcMsecToSamps(WOBBLE_DELAY_FLOOR_MAX + WOBBLE_DELAY_DEPTH));
    mWobbleDelayRight.setMaximumDelay(calcMsecToSamps(WOBBLE_DELAY_FLOOR_MAX + WOBBLE_DELAY_DEPTH));

    // Anti-alias FIR filter: Kaiser at 17 kHz cutoff, 3 kHz transition, 40 db attenuation @ 88.2k
    std::vector<stk::StkFloat> antiAliasFirCoeffs{
0.001094639609000579,
0.001506949002127016,
-0.000323940940369343,
-0.002404097801997796,
-0.001532782651968524,
0.002012726784394173,
0.003680711261579477,
0.000312787666705856,
-0.004545201300386254,
-0.004029760356383990,
0.002610630805991986,
0.007183607123039094,
0.002379410960134064,
-0.007142557727711353,
-0.008592438521945720,
0.002152258443407898,
0.012376178380798184,
0.007037989419697325,
-0.009855446816186613,
-0.016504874168315115,
-0.000693390471431345,
0.020150211174998132,
0.016847333575738324,
-0.012249133793767698,
-0.031566011947691486,
-0.009508849668427006,
0.034548474524006327,
0.042029270218467904,
-0.013895764948067421,
-0.077098173240771409,
-0.049393598970361927,
0.104126227497102791,
0.296936924668179780,
0.384699384420828516,
0.296936924668179780,
0.104126227497102791,
-0.049393598970361927,
-0.077098173240771409,
-0.013895764948067421,
0.042029270218467904,
0.034548474524006327,
-0.009508849668427006,
-0.031566011947691486,
-0.012249133793767698,
0.016847333575738324,
0.020150211174998132,
-0.000693390471431345,
-0.016504874168315115,
-0.009855446816186613,
0.007037989419697325,
0.012376178380798184,
0.002152258443407898,
-0.008592438521945720,
-0.007142557727711355,
0.002379410960134063,
0.007183607123039094,
0.002610630805991986,
-0.004029760356383990,
-0.004545201300386254,
0.000312787666705856,
0.003680711261579477,
0.002012726784394173,
-0.001532782651968523,
-0.002404097801997795,
-0.000323940940369343,
0.001506949002127016,
0.001094639609000579,
    };

    mAntiAliasLeft.setCoefficients(antiAliasFirCoeffs);
    mAntiAliasLeft.setSampleRate(mFs * 2);

    mAntialiasRight.setCoefficients(antiAliasFirCoeffs);
    mAntialiasRight.setSampleRate(mFs * 2);
}

void KartikCPFinalVH20AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KartikCPFinalVH20AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float KartikCPFinalVH20AudioProcessor::distort(float x) {
    float distWeight = distortStrength;
    float unitWeight = 1 - distWeight;
    float slope;
    float intercept;

    x *= distortDrive;

    // Tube waveshape
    if (x < -0.5) {
        slope = 3.0 / 4.0;
        intercept = -1.0 / 4.0;
    }
    else if (x < -0.125) {
        slope = 4.0 / 3.0;
        intercept = 1.0/24.0;
    }
    else if (x < 0) {
        slope = 1;
        intercept = 0;
    }
    else if (x < 0.5) {
        slope = 5.0 / 4.0;
        intercept = 0;
    }
    else if (x < 0.875) {
        slope = 1;
        intercept = 1.0 / 8.0;
    }
    else {
        slope = 0;
        intercept = 1;
    }

    // Interpolate between unit waveshape and distortion waveshape depending on computed drive parameter
    return ((slope * distWeight + 1 * unitWeight) * x + (intercept * distWeight + 0 * unitWeight)) / distortDriveGainCompensation;

}

void KartikCPFinalVH20AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    calcAlgorithmParams();

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    // Oversample x2
    auto n = buffer.getNumSamples();
    auto nOversampled = n * 2;

    auto bufferOversampled = juce::AudioBuffer<float>(2, nOversampled);
    auto* leftOversampled = bufferOversampled.getWritePointer(0);
    auto* rightOversampled = bufferOversampled.getWritePointer(1);

    for (int samp = 0; samp < n; samp++) {
        // Anti-image and interpolate
        leftOversampled[samp * 2] = mInterpolLpfLeft.tick(left[samp] * 2);
        leftOversampled[samp * 2 + 1] = mInterpolLpfLeft.tick(0);

        rightOversampled[samp * 2] = mInterpolLpfRight.tick(right[samp] * 2);
        rightOversampled[samp * 2 + 1] = (mInterpolLpfRight.tick(0));
    }

    // Process oversampled
    for (int samp = 0; samp < nOversampled; samp++) {

        float wetLeft = leftOversampled[samp];
        float wetRight = rightOversampled[samp];

        // Distort
        wetLeft = distort(wetLeft);
        wetRight = distort(wetRight);

        // Noise
        float noiseLeft = (noiseGen.nextFloat() * noiseAmplDoubled - noiseAmpl);
        float noiseRight = (noiseGen.nextFloat() * noiseAmplDoubled - noiseAmpl);

        // LPF for pink
        if (mDustTypeParam->getIndex() == 1) {
            noiseLeft = mPinkLpfLeft.tick(noiseLeft) * 8;
            noiseRight = mPinkLpfRight.tick(noiseRight) * 8;
        }

        wetLeft += noiseLeft;
        wetRight += noiseRight;

        // Wobble (modulated)
        wetLeft = mWobbleDelayLeft.tick(wetLeft);
        wetRight = mWobbleDelayRight.tick(wetRight);

        float lfoPos = (mLfo.tick() + 1) / 2;                        // [-1, 1] -> [0, 1]
        float delayTime = wobbleDelayRange.convertFrom0to1(lfoPos);
        mWobbleDelayLeft.setDelay(calcMsecToSamps(delayTime));
        mWobbleDelayRight.setDelay(calcMsecToSamps(delayTime));

        // Anti-alias
        wetLeft = mAntiAliasLeft.tick(wetLeft);
        wetRight = mAntialiasRight.tick(wetRight);

        // Downsample
        if (samp % 2 == 0) {
             left[samp/2] = wetLeft;
             right[samp/2] = wetRight;
        }
    }
}

void KartikCPFinalVH20AudioProcessor::calcAlgorithmParams() {
    noiseAmpl = mDustAmountParam->convertTo0to1(mDustAmountParam->get()) * 0.04; // Noise amplitude ranges from [0, 0.04]
    noiseAmplDoubled = noiseAmpl * 2;

    float gritAmount = mGritAmountParam->convertTo0to1(mGritAmountParam->get());
    distortStrength = gritAmount * 2; // Waveshape weight ranges from [0, 2]
    distortDrive = 1 + gritAmount * 5; // Input multiplier ranges from [1, 6]
    distortDriveGainCompensation = 1 + gritAmount * 3; // Output divisor ranges from [1, 4]

    delayMin = mWearAmountParam->convertTo0to1(mWearAmountParam->get()) * 20; 
    // Avg delay is either 0 or ranges from [1.5, 21.5]
    wobbleDelayRange = juce::NormalisableRange<float>(0, delayMin > 0 ? WOBBLE_DELAY_DEPTH + delayMin : 0); 
}

int KartikCPFinalVH20AudioProcessor::calcMsecToSamps(float msec) {
    return msec * mFs / 1000;
}

//==============================================================================
bool KartikCPFinalVH20AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KartikCPFinalVH20AudioProcessor::createEditor()
{
    return new KartikCPFinalVH20AudioProcessorEditor (*this);
}

//==============================================================================
void KartikCPFinalVH20AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void KartikCPFinalVH20AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KartikCPFinalVH20AudioProcessor();
}
