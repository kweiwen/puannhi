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

        auto mixCtrl = mMixCtrl[channel].process(mMix->get());
        auto speedCtrl = mSpeedCtrl[channel].process(mSpeed->get());
        auto decayCtrl = mDecayCtrl[channel].process(mDecay->get());
        auto dampCtrl = mDampCtrl[channel].process(mDamp->get());
        auto colorCtrl = mColorCtrl[channel].process(mColor->get());
        auto depthCtrl = mDepthCtrl[channel].process(mDepth->get());

        mCoefficient.setParameter(colorCtrl, getSampleRate(), 0, 0, 0);
        mFilter_1[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0], 0, 0, mCoefficient.getCoefficients()[3], mCoefficient.getCoefficients()[4], 0));
        mFilter_2[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0], 0, 0, mCoefficient.getCoefficients()[3], mCoefficient.getCoefficients()[4], 0));
        mFilter_3[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0], 0, 0, mCoefficient.getCoefficients()[3], mCoefficient.getCoefficients()[4], 0));
        mFilter_4[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0], 0, 0, mCoefficient.getCoefficients()[3], mCoefficient.getCoefficients()[4], 0));
        
        auto rad_0 = 0.125 * TWO_PI;
        auto rad_1 = 0.125 * TWO_PI;

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // ..do something to the data...
            auto drySignal = channelData[sample];

            // ramping process 
            auto preDelayCtrl = mPreDelayCtrl[channel].process(mPreDelay->get()) / 1000;
            auto sizeCtrl = mSizeCtrl[channel].process(mSize->get());

            auto modulation_1 = modulator_1[channel].process(speedCtrl, getSampleRate(), 0.0f, 0.0f);
            auto modulation_2 = modulator_2[channel].process(speedCtrl * 1.005f, getSampleRate(), 0, 0.25f * TWO_PI);
            auto modulation_3 = modulator_3[channel].process(speedCtrl * 1.005f * 1.005f, getSampleRate(), 0, 0.50f * TWO_PI);
            auto modulation_4 = modulator_4[channel].process(speedCtrl * 1.005f * 1.005f * 1.005f, getSampleRate(), 0, 0.75f * TWO_PI);

            feedbackLoop_1[channel] = CB_1[channel].readBuffer((887.0f + modulation_1 * depthCtrl), true);
            feedbackLoop_2[channel] = CB_2[channel].readBuffer((1913.0f + modulation_2 * depthCtrl), true);
            feedbackLoop_3[channel] = CB_3[channel].readBuffer((3967.0f + modulation_3 * depthCtrl), true);
            feedbackLoop_4[channel] = CB_4[channel].readBuffer((8053.0f + modulation_4 * depthCtrl), true);
            
            auto temp1 = feedbackLoop_1[channel] * cosf(rad_0) - feedbackLoop_2[channel] * sinf(rad_0);
            auto temp2 = feedbackLoop_1[channel] * sinf(rad_0) + feedbackLoop_2[channel] * cosf(rad_0);
            auto temp3 = feedbackLoop_3[channel] * cosf(rad_0) - feedbackLoop_4[channel] * sinf(rad_0);
            auto temp4 = feedbackLoop_3[channel] * sinf(rad_0) + feedbackLoop_4[channel] * cosf(rad_0);
            
            auto output_1 = temp1 * cosf(rad_1) - temp3 * sinf(rad_1);
            auto output_2 = temp1 * sinf(rad_1) + temp3 * cosf(rad_1);
            auto output_3 = temp2 * cosf(rad_1) - temp4 * sinf(rad_1);
            auto output_4 = temp2 * sinf(rad_1) + temp4 * cosf(rad_1);

            CB_1[channel].writeBuffer(drySignal + output_1 * sizeCtrl);
            CB_2[channel].writeBuffer(drySignal + output_2 * sizeCtrl);
            CB_3[channel].writeBuffer(mFilter_1[channel].processSingleSampleRaw(output_3 * sizeCtrl));
            CB_4[channel].writeBuffer(mFilter_2[channel].processSingleSampleRaw(output_4 * sizeCtrl));

            channelData[sample] = PreDelay[channel].process(output_1 * 0.25f, preDelayCtrl * getSampleRate() + 1, 0, 1) * mixCtrl + drySignal * (1 - mixCtrl);
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
