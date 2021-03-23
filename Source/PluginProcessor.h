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

//==============================================================================
/**
*/

const bool debug = false;

class CircularBufferAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CircularBufferAudioProcessor();
    ~CircularBufferAudioProcessor() override;

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
//    std::unique_ptr<CircularBuffer<double>[]> mCircularBuffer;
    std::unique_ptr<DelayFeedback<float>[]> mCircularBuffer;
    std::vector<juce::IIRFilter> mFilter;
    
    std::vector<ParameterSmooth> mFeedbackCtrl;
    std::vector<ParameterSmooth> mTimeCtrl;
    std::vector<ParameterSmooth> mMixCtrl;
    std::vector<ParameterSmooth> mCutOffCtrl;

    juce::AudioParameterFloat* mFeedback;
    juce::AudioParameterFloat* mTime;
    juce::AudioParameterFloat* mMix;
    juce::AudioParameterFloat* mCutOff;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularBufferAudioProcessor);
};
