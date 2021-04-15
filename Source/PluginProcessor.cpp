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
    addParameter (mMix      = new juce::AudioParameterFloat  ("0x00",  "Mixing", 0.01f,  1.00f,  1.00f));
    addParameter (mPitch    = new juce::AudioParameterInt    ("0x01",  "Pitch",  -12,    12,     0));
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
    
    mCircularBuffer_1.reset(new DelayFeedback<float>[getTotalNumInputChannels()]);
    mCircularBuffer_2.reset(new DelayFeedback<float>[getTotalNumInputChannels()]);

    for (int index = 0; index < getTotalNumInputChannels(); index++)
    {
        mCircularBuffer_1[index].digitalDelayLine.createCircularBuffer(sampleRate * 0.05);
        mCircularBuffer_1[index].digitalDelayLine.flushBuffer();
        
        mCircularBuffer_2[index].digitalDelayLine.createCircularBuffer(sampleRate * 0.05);
        mCircularBuffer_2[index].digitalDelayLine.flushBuffer();
        
        mMixCtrl.push_back(ParameterSmooth());
        mMixCtrl[index].createCoefficients(sampleRate / 100, sampleRate);
        
        mPitchCtrl.push_back(ParameterSmooth());
        mPitchCtrl[index].createCoefficients(sampleRate / 100, sampleRate);

        modulator_1.push_back(Oscillator());
        modulator_2.push_back(Oscillator());
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
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // ..do something to the data...
            // start to smooth the parameter control
            auto mixCtrl = mMixCtrl[channel].process(mMix->get());
            auto pitchCtrl = mPitchCtrl[channel].process(mPitch->get());
            auto pitch = ((pow(2, (pitchCtrl / 12.0))) - 1) * -20;
            
            auto phasor_1 = modulator_1[channel].process(pitch, getSampleRate(), 5);
            auto phasor_2 = modulator_2[channel].process(pitch, getSampleRate(), 6);
            
            auto delay_line_modulation_1 = phasor_1 * getSampleRate() * 0.05;
            auto delay_line_modulation_2 = phasor_2 * getSampleRate() * 0.05;
            
            auto window_1 = ((cos(TWO_PI * phasor_1) * -1) + 1) / 2.0;
            auto window_2 = ((cos(TWO_PI * phasor_2) * -1) + 1) / 2.0;

            auto delay_1 = mCircularBuffer_1[channel].process(channelData[sample], delay_line_modulation_1, 0, mixCtrl) * window_1;
            auto delay_2 = mCircularBuffer_2[channel].process(channelData[sample], delay_line_modulation_2, 0, mixCtrl) * window_2;
            
            channelData[sample] = delay_1 + delay_2;
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
