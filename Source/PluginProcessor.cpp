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
    addParameter    (mPreDelay   = new juce::AudioParameterFloat    ("0x01",    "Pre-Delay",  0.00f,  50.0f,  0.00f));
    addParameter    (mColor      = new juce::AudioParameterFloat    ("0x02",    "Color",      0,      7500,   1500));
    addParameter    (mDamp       = new juce::AudioParameterFloat    ("0x03",    "Damping",    0.00f,  1.00f,  0.50f));
    addParameter    (mDecay      = new juce::AudioParameterFloat    ("0x04",    "Decay",      0.00f,  1.05f,  0.50f));
    addParameter    (mSpread     = new juce::AudioParameterFloat    ("0x05",    "Spread",     0.00f,  1.00f,  0.50f));
    addParameter    (mSize       = new juce::AudioParameterFloat    ("0x06",    "Size",       0.01f,  1.00f,  0.50f));
    addParameter    (mSpeed      = new juce::AudioParameterFloat    ("0x07",    "Speed",      0.1f,   30.0f,  1.00f));
    addParameter    (mDepth      = new juce::AudioParameterFloat    ("0x08",    "Depth",      0.0f,   100.0f, 40.0f));
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
        CB_1[index].createCircularBuffer(8192);
        CB_1[index].flushBuffer();

        CB_2[index].createCircularBuffer(8192);
        CB_2[index].flushBuffer();

        CB_3[index].createCircularBuffer(8192);
        CB_3[index].flushBuffer();

        CB_4[index].createCircularBuffer(8192);
        CB_4[index].flushBuffer();

        PreDelay[index].digitalDelayLine.createCircularBuffer(sampleRate * 0.05);
        PreDelay[index].digitalDelayLine.flushBuffer();
        
        mMixCtrl.push_back(ParameterSmooth());
        mMixCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mPreDelayCtrl.push_back(ParameterSmooth());
        mPreDelayCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mDampCtrl.push_back(ParameterSmooth());
        mDampCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mColorCtrl.push_back(ParameterSmooth());
        mColorCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mDecayCtrl.push_back(ParameterSmooth());
        mDecayCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mSpreadCtrl.push_back(ParameterSmooth());
        mSpreadCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);
        
        mSizeCtrl.push_back(ParameterSmooth());
        mSizeCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mDepthCtrl.push_back(ParameterSmooth());
        mDepthCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mSpeedCtrl.push_back(ParameterSmooth());
        mSpeedCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mFilter_1.push_back(juce::IIRFilter());
        mFilter_2.push_back(juce::IIRFilter());
        mFilter_3.push_back(juce::IIRFilter());
        mFilter_4.push_back(juce::IIRFilter());
        
        feedbackLoop_1.push_back(0.0f);
        feedbackLoop_2.push_back(0.0f);
        feedbackLoop_3.push_back(0.0f);
        feedbackLoop_4.push_back(0.0f);

        modulator.push_back(Oscillator());
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {   
        auto* channelData = buffer.getWritePointer (channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // ..do something to the data...
            auto drySignal = channelData[sample];

            // ramping process 
            auto mixCtrl = mMixCtrl[channel].process(mMix->get());
            auto preDelayCtrl = mPreDelayCtrl[channel].process(mPreDelay->get()) / 1000;
            auto decayCtrl = mDecayCtrl[channel].process(mDecay->get());
            auto spreadCtrl = (mSpreadCtrl[channel].process(mSpread->get()) * 80.0 + 5.0) * TWO_PI / 360;
            auto sine = sin(spreadCtrl);
            auto cosine = cos(spreadCtrl);
            auto sizeCtrl = mSizeCtrl[channel].process(mSize->get());
            auto speedCtrl = mSpeedCtrl[channel].process(mSpeed->get());
            auto depthCtrl = mDepthCtrl[channel].process(mDepth->get());
            auto modulation = modulator[channel].process(speedCtrl, getSampleRate(), 0);
            auto dampCtrl = mDampCtrl[channel].process(mDamp->get());
            auto colorCtrl = mColorCtrl[channel].process(mColor->get());

            feedbackLoop_1[channel] = CB_1[channel].readBuffer(2819);
            feedbackLoop_2[channel] = CB_2[channel].readBuffer(3343);
            feedbackLoop_3[channel] = CB_3[channel].readBuffer(3581);
            feedbackLoop_4[channel] = CB_4[channel].readBuffer(4133);

            mCoefficient.setParameter(colorCtrl, getSampleRate(), 0, 0, 0);

            mFilter_1[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0],
                mCoefficient.getCoefficients()[1],
                mCoefficient.getCoefficients()[2],
                mCoefficient.getCoefficients()[3],
                mCoefficient.getCoefficients()[4],
                mCoefficient.getCoefficients()[5]));

            mFilter_2[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0],
                mCoefficient.getCoefficients()[1],
                mCoefficient.getCoefficients()[2],
                mCoefficient.getCoefficients()[3],
                mCoefficient.getCoefficients()[4],
                mCoefficient.getCoefficients()[5]));

            mFilter_3[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0],
                mCoefficient.getCoefficients()[1],
                mCoefficient.getCoefficients()[2],
                mCoefficient.getCoefficients()[3],
                mCoefficient.getCoefficients()[4],
                mCoefficient.getCoefficients()[5]));

            mFilter_4[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0],
                mCoefficient.getCoefficients()[1],
                mCoefficient.getCoefficients()[2],
                mCoefficient.getCoefficients()[3],
                mCoefficient.getCoefficients()[4],
                mCoefficient.getCoefficients()[5]));

            auto lpf_1 = mFilter_1[channel].processSingleSampleRaw(feedbackLoop_1[channel]);
            auto lpf_2 = mFilter_2[channel].processSingleSampleRaw(feedbackLoop_2[channel]);
            auto lpf_3 = mFilter_3[channel].processSingleSampleRaw(feedbackLoop_3[channel]);
            auto lpf_4 = mFilter_4[channel].processSingleSampleRaw(feedbackLoop_4[channel]);

            auto damp_output_1 = (lpf_1 - feedbackLoop_1[channel]) * dampCtrl;
            auto damp_output_2 = (lpf_2 - feedbackLoop_2[channel]) * dampCtrl;
            auto damp_output_3 = (lpf_3 - feedbackLoop_3[channel]) * dampCtrl;
            auto damp_output_4 = (lpf_4 - feedbackLoop_4[channel]) * dampCtrl;

            auto A = (damp_output_1 + feedbackLoop_1[channel]) * 0.5f * decayCtrl + drySignal;
            auto B = (damp_output_2 + feedbackLoop_2[channel]) * 0.5f * decayCtrl + drySignal;
            auto C = (damp_output_3 + feedbackLoop_3[channel]) * 0.5f * decayCtrl;
            auto D = (damp_output_4 + feedbackLoop_4[channel]) * 0.5f * decayCtrl;
                        
            auto output_1 = (A + B + C + D);
            auto output_2 = (A - B + C - D);
            auto output_3 = (A + B - C - D);
            auto output_4 = (A - B - C + D);

            CB_1[channel].writeBuffer(output_1);
            CB_2[channel].writeBuffer(output_2);
            CB_3[channel].writeBuffer(output_3);
            CB_4[channel].writeBuffer(output_4);

            channelData[sample] = PreDelay[channel].process(A, preDelayCtrl * getSampleRate() + 1, 0, 1) * mixCtrl + drySignal * (1 - mixCtrl);
        }
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
