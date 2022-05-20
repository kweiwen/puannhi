/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CircularBuffer.h"
#include "ParameterSmooth.h"
#include "DelayFeedback.h"
#include "FilterDesigner.h"
#include "Oscillator.h"
#include "DelayAPF.h"

//==============================================================================
/**
*/

const bool debug = false;

class PuannhiAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PuannhiAudioProcessor();
    ~PuannhiAudioProcessor() override;

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
    std::unique_ptr<DelayFeedback<float>[]> Pitch_CB_1;
    std::unique_ptr<DelayFeedback<float>[]> Picth_CB_2;
    std::unique_ptr<DelayAPF<float>[]> All_Pass_Delay;
    std::unique_ptr<CircularBuffer<float>[]> Pre_Delay;
    std::unique_ptr<CircularBuffer<float>[]> CB_1;
    std::unique_ptr<CircularBuffer<float>[]> CB_2;
    std::unique_ptr<CircularBuffer<float>[]> CB_3;
    std::unique_ptr<CircularBuffer<float>[]> CB_4;
    
    FilterDesigner mCoefficient;
    
    std::vector<float> feedbackLoop_1;
    std::vector<float> feedbackLoop_2;
    std::vector<float> feedbackLoop_3;
    std::vector<float> feedbackLoop_4;

    std::vector<juce::IIRFilter> mFilter_1;
    std::vector<juce::IIRFilter> mFilter_2;
    std::vector<juce::IIRFilter> mHighPass;

    std::vector<Oscillator> pitch_modulator_1;
    std::vector<Oscillator> pitch_modulator_2;
    std::vector<Oscillator> modulator_1;
    std::vector<Oscillator> modulator_2;
    std::vector<Oscillator> modulator_3;
    std::vector<Oscillator> modulator_4;
    std::vector<Oscillator> spread_osc;
    
    juce::AudioParameterInt* mPitch;
    juce::AudioParameterFloat* mMix;
    juce::AudioParameterFloat* mPreDelay;
    juce::AudioParameterFloat* mColor;
    juce::AudioParameterFloat* mSpread;
    juce::AudioParameterFloat* mFeedback;
    juce::AudioParameterFloat* mDecay;
    juce::AudioParameterFloat* mShimmer;
    juce::AudioParameterFloat* mDepth;
    
    std::vector<ParameterSmooth> mPitchCtrl;
    std::vector<ParameterSmooth> mMixCtrl;
    std::vector<ParameterSmooth> mPreDelayCtrl;
    std::vector<ParameterSmooth> mColorCtrl;
    std::vector<ParameterSmooth> mSpreadCtrl;
    std::vector<ParameterSmooth> mFeedbackCtrl;
    std::vector<ParameterSmooth> mDecayCtrl;
    std::vector<ParameterSmooth> mShimmerCtrl;
    std::vector<ParameterSmooth> mDepthCtrl;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PuannhiAudioProcessor);
};
