/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PuannhiAudioProcessor::PuannhiAudioProcessor()
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
    addParameter    (mMix        = new juce::AudioParameterFloat    ("0x00",    "Mixing",     0.00f,  1.00f,  0.50f));
    addParameter    (mPreDelay   = new juce::AudioParameterFloat    ("0x01",    "Pre-Delay",  0.00f,  150.0f, 0.00f));
    addParameter    (mColor      = new juce::AudioParameterFloat    ("0x02",    "Brightness", 150,    5000,   1000));
    addParameter    (mDamp       = new juce::AudioParameterFloat    ("0x03",    "Damping",    0.00f,  1.00f,  0.50f));
    addParameter    (mDecay      = new juce::AudioParameterFloat    ("0x04",    "Decay",      0.00f,  1.00f,  0.50f));
    addParameter    (mSize       = new juce::AudioParameterFloat    ("0x06",    "Size",       0.01f,  1.00f,  1.00f));
    addParameter    (mSpeed      = new juce::AudioParameterFloat    ("0x07",    "Speed",      0.1f,   4.00f,  1.00f));
    addParameter    (mDepth      = new juce::AudioParameterFloat    ("0x08",    "Depth",      0.0f,   100.0f, 40.0f));
    addParameter    (mGainFS     = new juce::AudioParameterFloat    ("0x09",    "GainFS",     -48.0f, 6.0f,   0.0f));
    addParameter    (mGainFA     = new juce::AudioParameterFloat    ("0x0A",    "GainFA",     -48.0f, 6.0f,   0.0f));
    addParameter    (mGainRA     = new juce::AudioParameterFloat    ("0x0B",    "GainRA",     -48.0f, 6.0f,   0.0f));
    updateFFTsize(2048);
}

PuannhiAudioProcessor::~PuannhiAudioProcessor()
{
}

//==============================================================================
const juce::String PuannhiAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PuannhiAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PuannhiAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PuannhiAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PuannhiAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PuannhiAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PuannhiAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PuannhiAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PuannhiAudioProcessor::getProgramName (int index)
{
    return {};
}

void PuannhiAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PuannhiAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    updateFFTsize(2048);

    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    CB_1.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_2.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_3.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_4.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);

    PreDelay.reset(new DelayFeedback<float>[getTotalNumInputChannels()]);

    mCoefficient.model = 4;

    for (int index = 0; index < getTotalNumInputChannels(); index++)
    {
        CB_1[index].createCircularBuffer(4096);
        CB_1[index].flushBuffer();

        CB_2[index].createCircularBuffer(4096);
        CB_2[index].flushBuffer();

        CB_3[index].createCircularBuffer(4096);
        CB_3[index].flushBuffer();

        CB_4[index].createCircularBuffer(4096);
        CB_4[index].flushBuffer();

        PreDelay[index].digitalDelayLine.createCircularBuffer(8192);
        PreDelay[index].digitalDelayLine.flushBuffer();
        
        mMixCtrl.push_back(ParameterSmooth());
        mMixCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mPreDelayCtrl.push_back(ParameterSmooth());
        mPreDelayCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mDampCtrl.push_back(ParameterSmooth());
        mDampCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mColorCtrl.push_back(ParameterSmooth());
        mColorCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mDecayCtrl.push_back(ParameterSmooth());
        mDecayCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mSizeCtrl.push_back(ParameterSmooth());
        mSizeCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mDepthCtrl.push_back(ParameterSmooth());
        mDepthCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mSpeedCtrl.push_back(ParameterSmooth());
        mSpeedCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mGainFSCtrl.push_back(ParameterSmooth());
        mGainFSCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mGainFACtrl.push_back(ParameterSmooth());
        mGainFACtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mGainRACtrl.push_back(ParameterSmooth());
        mGainRACtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mFilter_1.push_back(juce::IIRFilter());
        mFilter_2.push_back(juce::IIRFilter());
        mFilter_3.push_back(juce::IIRFilter());
        mFilter_4.push_back(juce::IIRFilter());
        
        feedbackLoop_1.push_back(0.0f);
        feedbackLoop_2.push_back(0.0f);
        feedbackLoop_3.push_back(0.0f);
        feedbackLoop_4.push_back(0.0f);

        modulator_1.push_back(Oscillator());
        modulator_2.push_back(Oscillator());
        modulator_3.push_back(Oscillator());
        modulator_4.push_back(Oscillator());


    }
}

void PuannhiAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PuannhiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PuannhiAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // number of samples per buffer
    const int numSamples = buffer.getNumSamples();

    // input channels
    auto channelData_left = buffer.getReadPointer(0);
    auto channelData_right = buffer.getReadPointer(1);

    // output channels
    float* const ChannelDataOut_1 = buffer.getWritePointer(0);
    float* const ChannelDataOut_2 = buffer.getWritePointer(1);
    float* const ChannelDataOut_3 = buffer.getWritePointer(2);
    float* const ChannelDataOut_4 = buffer.getWritePointer(3);
    float* const ChannelDataOut_5 = buffer.getWritePointer(4);

    // main STFT analysis loop
    for (int sample = 0; sample < numSamples; ++sample) {
        // input sample per channel
        timeDomainBuffer_left[sample].real(channelData_left[sample]);
        timeDomainBuffer_left[sample].imag(0.0f);

        timeDomainBuffer_right[sample].real(channelData_right[sample]);
        timeDomainBuffer_right[sample].imag(0.0f);
    }

    // FFT - Analysis
    fft->perform(timeDomainBuffer_left, frequencyDomainBuffer_left, false);
    fft->perform(timeDomainBuffer_right, frequencyDomainBuffer_right, false);

    // declare const var for Direct Component calculation
    // exp(i*0.6*pi) = cos(0.6*pi) + i*sin(0.6*pi) = tmpExp
    std::complex<float> tmpExp(-0.30901699437, -0.30901699437);

    auto gainFSCtrl = pow(10, mGainFSCtrl[0].process(mGainFS->get()) / 20 );
    auto gainFACtrl = pow(10, mGainFACtrl[0].process(mGainFA->get()) / 20 );
    auto gainRACtrl = pow(10, mGainRACtrl[0].process(mGainRA->get()) / 20 );

    // Main Upmixing Processing Loop
    for (int sample = 0; sample < numSamples; ++sample) {
        // calculation of Panning Coefficients
        float aL = abs(frequencyDomainBuffer_left[sample]) / sqrt(abs(frequencyDomainBuffer_left[sample]) * abs(frequencyDomainBuffer_left[sample]) + abs(frequencyDomainBuffer_right[sample]) * abs(frequencyDomainBuffer_right[sample]));
        float aR = abs(frequencyDomainBuffer_right[sample]) / sqrt(abs(frequencyDomainBuffer_left[sample]) * abs(frequencyDomainBuffer_left[sample]) + abs(frequencyDomainBuffer_right[sample]) * abs(frequencyDomainBuffer_right[sample]));


        // calculation of Direct Component
        Direct[sample] = (frequencyDomainBuffer_left[sample] * tmpExp - frequencyDomainBuffer_right[sample]) / (aL * tmpExp - aR);

        // calculation of Ambient(L&R) Components
        DL[sample] = Direct[sample] * aL;
        DR[sample] = Direct[sample] * aR;
        NL[sample] = frequencyDomainBuffer_left[sample] - DL[sample];
        NR[sample] = frequencyDomainBuffer_right[sample] - DR[sample];

        // up-mixing Direct Component
        // sqrt(0.5) = 0.70710678118
        DC_mag[sample] = (float)0.70710678118 * (abs(DL[sample] + DR[sample]) - abs(DL[sample] - DR[sample])); // Center Channel Magnitude
        float nl_dbl_min = std::numeric_limits<float>::min();
        DC[sample] = ((DL[sample] + DR[sample]) * DC_mag[sample]) / (abs(DL[sample] + DR[sample]) + nl_dbl_min); // Calculation of Center Channel
        CL[sample] = DL[sample] - DC[sample] * (float)0.70710678118; // Calculation of Left & Right Channels
        CR[sample] = DR[sample] - DC[sample] * (float)0.70710678118; // Calculation of Left & Right Channels
    }

    // iFFT - Synthesis
    fft->perform(CL, timeDomain_DL, true);
    fft->perform(CR, timeDomain_DR, true);
    fft->perform(DC, timeDomain_DC, true);
    fft->perform(NL, timeDomain_NL, true);
    fft->perform(NR, timeDomain_NR, true);

    // main STFT synthesis loop
    for (int sample = 0; sample < numSamples; ++sample) {
        ChannelDataOut_1[sample] = (timeDomain_DL[sample].real() * gainFSCtrl) + (timeDomain_NL[sample].real() * gainFACtrl);
        ChannelDataOut_2[sample] = (timeDomain_DR[sample].real() * gainFSCtrl) + (timeDomain_NR[sample].real() * gainFACtrl);
        ChannelDataOut_3[sample] = timeDomain_DC[sample].real();
        ChannelDataOut_4[sample] = timeDomain_NL[sample].real() * gainRACtrl;
        ChannelDataOut_5[sample] = timeDomain_NR[sample].real() * gainRACtrl;
    }
}

//==============================================================================
bool PuannhiAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PuannhiAudioProcessor::createEditor()
{
    return new PuannhiAudioProcessorEditor (*this);
//    return nullptr;
}

//==============================================================================
void PuannhiAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PuannhiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PuannhiAudioProcessor();
}
