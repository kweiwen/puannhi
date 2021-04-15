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
    std::unique_ptr<DelayFeedback<float>[]> mCircularBuffer_1;
    std::unique_ptr<DelayFeedback<float>[]> mCircularBuffer_2;
    
    std::vector<Oscillator> modulator_1;
    std::vector<Oscillator> modulator_2;
    
    std::vector<ParameterSmooth> mMixCtrl;
    std::vector<ParameterSmooth> mPitchCtrl;

    juce::AudioParameterFloat* mMix;
    juce::AudioParameterInt* mPitch;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularBufferAudioProcessor);
};
