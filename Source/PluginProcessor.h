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


    // declare custom methods
    void updateFFTsize(const int newFftSize)
    {
        fftSize = newFftSize;
        fft = new juce::dsp::FFT(log2(fftSize));

        timeDomainBuffer_left.realloc(fftSize);
        timeDomainBuffer_left.clear(fftSize);
        timeDomainBuffer_right.realloc(fftSize);
        timeDomainBuffer_right.clear(fftSize);

        frequencyDomainBuffer_left.realloc(fftSize);
        frequencyDomainBuffer_left.clear(fftSize);
        frequencyDomainBuffer_right.realloc(fftSize);
        frequencyDomainBuffer_right.clear(fftSize);

        NL.realloc(fftSize);
        NL.clear(fftSize);
        NR.realloc(fftSize);
        NR.clear(fftSize);
        Direct.realloc(fftSize);
        Direct.clear(fftSize);
        DL.realloc(fftSize);
        DL.clear(fftSize);
        DR.realloc(fftSize);
        DR.clear(fftSize);
        DC_mag.realloc(fftSize);
        DC_mag.clear(fftSize);
        DC.realloc(fftSize);
        DC.clear(fftSize);
        CL.realloc(fftSize);
        CL.clear(fftSize);
        CR.realloc(fftSize);
        CR.clear(fftSize);

        timeDomain_DL.realloc(fftSize);
        timeDomain_DL.clear(fftSize);
        timeDomain_DR.realloc(fftSize);
        timeDomain_DR.clear(fftSize);
        timeDomain_DC.realloc(fftSize);
        timeDomain_DC.clear(fftSize);
        timeDomain_NL.realloc(fftSize);
        timeDomain_NL.clear(fftSize);
        timeDomain_NR.realloc(fftSize);
        timeDomain_NR.clear(fftSize);
    }

private:
    int fftSize;
    juce::ScopedPointer<juce::dsp::FFT> fft;

    juce::HeapBlock<juce::dsp::Complex<float>> timeDomainBuffer_left;
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomainBuffer_right;
    juce::HeapBlock<juce::dsp::Complex<float>> frequencyDomainBuffer_left;
    juce::HeapBlock<juce::dsp::Complex<float>> frequencyDomainBuffer_right;

    juce::HeapBlock<juce::dsp::Complex<float>> Direct;
    juce::HeapBlock<juce::dsp::Complex<float>> NL;
    juce::HeapBlock<juce::dsp::Complex<float>> NR;
    juce::HeapBlock<juce::dsp::Complex<float>> DL;
    juce::HeapBlock<juce::dsp::Complex<float>> DR;
    juce::HeapBlock<juce::dsp::Complex<float>> DC_mag;
    juce::HeapBlock<juce::dsp::Complex<float>> DC;
    juce::HeapBlock<juce::dsp::Complex<float>> CL;
    juce::HeapBlock<juce::dsp::Complex<float>> CR;

    juce::HeapBlock<juce::dsp::Complex<float>> timeDomain_DL;
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomain_DR;
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomain_DC;
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomain_NL;
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomain_NR;

    std::unique_ptr<CircularBuffer<float>[]> CB_1;
    std::unique_ptr<CircularBuffer<float>[]> CB_2;
    std::unique_ptr<CircularBuffer<float>[]> CB_3;
    std::unique_ptr<CircularBuffer<float>[]> CB_4;

    std::unique_ptr<DelayFeedback<float>[]> PreDelay;

    FilterDesigner mCoefficient;
    
    std::vector<float> feedbackLoop_1;
    std::vector<float> feedbackLoop_2;
    std::vector<float> feedbackLoop_3;
    std::vector<float> feedbackLoop_4;

    std::vector<juce::IIRFilter> mFilter_1;
    std::vector<juce::IIRFilter> mFilter_2;
    std::vector<juce::IIRFilter> mFilter_3;
    std::vector<juce::IIRFilter> mFilter_4;

    std::vector<Oscillator> modulator_1;
    std::vector<Oscillator> modulator_2;
    std::vector<Oscillator> modulator_3;
    std::vector<Oscillator> modulator_4;

    juce::AudioParameterFloat* mMix;
    juce::AudioParameterFloat* mPreDelay;
    juce::AudioParameterFloat* mColor;
    juce::AudioParameterFloat* mDamp;
    juce::AudioParameterFloat* mDecay;
    juce::AudioParameterFloat* mSize;
    juce::AudioParameterFloat* mSpeed;
    juce::AudioParameterFloat* mDepth;
    juce::AudioParameterFloat* mGainFS;
    juce::AudioParameterFloat* mGainFA;
    juce::AudioParameterFloat* mGainRA;

    std::vector<ParameterSmooth> mMixCtrl;
    std::vector<ParameterSmooth> mPreDelayCtrl;
    std::vector<ParameterSmooth> mColorCtrl;
    std::vector<ParameterSmooth> mDampCtrl;
    std::vector<ParameterSmooth> mDecayCtrl;
    std::vector<ParameterSmooth> mSizeCtrl;
    std::vector<ParameterSmooth> mSpeedCtrl;
    std::vector<ParameterSmooth> mDepthCtrl;
    std::vector<ParameterSmooth> mGainFSCtrl;
    std::vector<ParameterSmooth> mGainFACtrl;
    std::vector<ParameterSmooth> mGainRACtrl;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PuannhiAudioProcessor);
};
