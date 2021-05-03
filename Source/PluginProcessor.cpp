/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularBufferAudioProcessor::CircularBufferAudioProcessor()
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
    addParameter    (mMix        = new juce::AudioParameterFloat  ("0x00",    "Mixing",     0.00f,  1.00f,  0.50f));
    addParameter    (mDamp       = new juce::AudioParameterFloat  ("0x01",    "Damping",    0.00f,  1.00f,  0.50f));
    addParameter    (mDecay      = new juce::AudioParameterFloat  ("0x02",    "Decay",      0.00f,  1.00f,  0.50f));
    addParameter    (mSpread     = new juce::AudioParameterFloat  ("0x03",    "Spread",     0.00f,  1.00f,  0.50f));
    addParameter    (mDensity    = new juce::AudioParameterFloat  ("0x04",    "Density",    0.00f,  0.70f,  0.50f));
    
    root2 = std::sqrt(2.f);
}

CircularBufferAudioProcessor::~CircularBufferAudioProcessor()
{
}

//==============================================================================
const juce::String CircularBufferAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CircularBufferAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CircularBufferAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CircularBufferAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CircularBufferAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CircularBufferAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CircularBufferAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CircularBufferAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CircularBufferAudioProcessor::getProgramName (int index)
{
    return {};
}

void CircularBufferAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CircularBufferAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    CB_1.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_2.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_3.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_4.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);

    APF_1.reset(new DelayAPF<float>[getTotalNumInputChannels()]);
    APF_2.reset(new DelayAPF<float>[getTotalNumInputChannels()]);
    APF_3.reset(new DelayAPF<float>[getTotalNumInputChannels()]);
    APF_4.reset(new DelayAPF<float>[getTotalNumInputChannels()]);

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

        APF_1[index].digitalDelayLine.createCircularBuffer(8192);
        APF_1[index].digitalDelayLine.flushBuffer();

        APF_2[index].digitalDelayLine.createCircularBuffer(8192);
        APF_2[index].digitalDelayLine.flushBuffer();

        APF_3[index].digitalDelayLine.createCircularBuffer(8192);
        APF_3[index].digitalDelayLine.flushBuffer();

        APF_4[index].digitalDelayLine.createCircularBuffer(8192);
        APF_4[index].digitalDelayLine.flushBuffer();
        
        mMixCtrl.push_back(ParameterSmooth());
        mMixCtrl[index].createCoefficients(sampleRate / 8000, sampleRate);

        mDampCtrl.push_back(ParameterSmooth());
        mDampCtrl[index].createCoefficients(sampleRate / 8000, sampleRate);

        mDecayCtrl.push_back(ParameterSmooth());
        mDecayCtrl[index].createCoefficients(sampleRate / 8000, sampleRate);

        mSpreadCtrl.push_back(ParameterSmooth());
        mSpreadCtrl[index].createCoefficients(sampleRate / 8000, sampleRate);
        
        mDensityCtrl.push_back(ParameterSmooth());
        mDensityCtrl[index].createCoefficients(sampleRate / 8000, sampleRate);

        mFilter_1.push_back(juce::IIRFilter());
        mFilter_2.push_back(juce::IIRFilter());
        
        feedbackLoop_1.push_back(0.0f);
        feedbackLoop_2.push_back(0.0f);
        feedbackLoop_3.push_back(0.0f);
        feedbackLoop_4.push_back(0.0f);
    }
}

void CircularBufferAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CircularBufferAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CircularBufferAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
        auto mixCtrl = mMixCtrl[channel].process(mMix->get());
        auto dampCtrl = mDampCtrl[channel].process(mDamp->get());
        auto decayCtrl = mDecayCtrl[channel].process(mDecay->get());
        auto spreadCtrl = (mSpreadCtrl[channel].process(mSpread->get()) * 60 + 15) * TWO_PI / 360;
        auto densityCtrl = mDensityCtrl[channel].process(mDensity->get());

        auto b0 = (1 - dampCtrl);
        auto a1 = -dampCtrl;
        
        mFilter_1[channel].setCoefficients(juce::IIRCoefficients(b0, 0.0f, 0.0f, 1, a1, 0.0f));
        mFilter_2[channel].setCoefficients(juce::IIRCoefficients(b0, 0.0f, 0.0f, 1, a1, 0.0f));
        
        auto sine = sin(spreadCtrl);
        auto cosine = cos(spreadCtrl);
        
        auto* channelData = buffer.getWritePointer (channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // ..do something to the data...
            auto drySignal = channelData[sample];

            auto lpf_1 = drySignal + feedbackLoop_1[channel] * decayCtrl;
            auto lpf_2 = drySignal + feedbackLoop_3[channel] * decayCtrl;
            auto lpf_3 = mFilter_1[channel].processSingleSampleRaw(feedbackLoop_2[channel] * decayCtrl);
            auto lpf_4 = mFilter_2[channel].processSingleSampleRaw(feedbackLoop_4[channel] * decayCtrl);
            lpf_3 = mFilter_1[channel].processSingleSampleRaw(lpf_3);
            lpf_4 = mFilter_2[channel].processSingleSampleRaw(lpf_4);
            
//            auto apf_1 = APF_1[channel].processSchroeder(lpf_1, 137, densityCtrl);
//            auto apf_2 = APF_1[channel].processSchroeder(lpf_2, 139, densityCtrl);
//            auto apf_3 = APF_1[channel].processSchroeder(lpf_3, 521, densityCtrl);
//            auto apf_4 = APF_1[channel].processSchroeder(lpf_4, 523, densityCtrl);
            
            auto A = CB_1[channel].readBuffer(1951);
            auto B = CB_2[channel].readBuffer(2992);
            auto C = CB_3[channel].readBuffer(4761);
            auto D = CB_4[channel].readBuffer(6480);
            
            CB_1[channel].writeBuffer(lpf_1);
            CB_2[channel].writeBuffer(lpf_2);
            CB_3[channel].writeBuffer(lpf_3);
            CB_4[channel].writeBuffer(lpf_4);
            
            channelData[sample] = (A+ B+ C + D) * mixCtrl + drySignal * (1 - mixCtrl);
            
            auto M1 = A * root2 * 0.5 - B * root2 * 0.5;
            auto M2 = A * root2 * 0.5 + B * root2 * 0.5;
            auto M3 = C * root2 * 0.5 - D * root2 * 0.5;
            auto M4 = C * root2 * 0.5 + D * root2 * 0.5;
            
            feedbackLoop_1[channel] = (M1 * sine - M3 * cosine);
            feedbackLoop_2[channel] = (M1 * cosine + M3 * sine);
            feedbackLoop_3[channel] = (M2 * sine - M4 * cosine);
            feedbackLoop_4[channel] = (M2 * cosine + M4 * sine);
        }
    }
}

//==============================================================================
bool CircularBufferAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CircularBufferAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
//    return nullptr;
}

//==============================================================================
void CircularBufferAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CircularBufferAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CircularBufferAudioProcessor();
}
