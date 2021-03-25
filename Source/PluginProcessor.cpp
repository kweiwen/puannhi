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
    //addParameter (mFeedback    = new juce::AudioParameterFloat ("feedback", "Feedback", 0.00f,  1.00f,      0.50f));
    //addParameter (mTime        = new juce::AudioParameterFloat ("time",     "Time",     0.01f,  1.00f,      0.50f));
    addParameter (mCutOff      = new juce::AudioParameterFloat  ("0x00",  "Frequency Cut-Off",   20.0f,  2500.0f,    1200.0f));
    addParameter (mResonance   = new juce::AudioParameterFloat  ("0x01",  "Resonance", juce::NormalisableRange<float>(0.1f, 18.0f, 0.1f), 1.0f));
    addParameter (mSpeed       = new juce::AudioParameterInt    ("0x02",  "Modulation Speed",    1,      10,         1));
    addParameter (mAmount      = new juce::AudioParameterInt    ("0x03",  "Modulation Amount",   100,    1000,       100));
    addParameter (mMix         = new juce::AudioParameterFloat  ("0x04",  "Mixing",              0.01f,  1.00f,      0.50f));
    addParameter (mFilterType  = new juce::AudioParameterChoice ("0x05",  "Filter Type",         {"FLAT","LPF", "BPF", "HPF"}, 0));
    addParameter (mOscType     = new juce::AudioParameterChoice ("0x06",  "Oscillator Type",     {"Sine", "Triangle", "Sawtooth","Trapezoid","Square"}, 0));
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
    
    mCircularBuffer.reset(new DelayFeedback<float>[getTotalNumInputChannels()]);

    for (int index = 0; index < getTotalNumInputChannels(); index++)
    {
        mCircularBuffer[index].digitalDelayLine.createCircularBuffer(sampleRate);
        mCircularBuffer[index].digitalDelayLine.flushBuffer();
        
        mTimeCtrl.push_back(ParameterSmooth());
        mTimeCtrl[index].createCoefficients(sampleRate / 100, sampleRate);
        
        mMixCtrl.push_back(ParameterSmooth());
        mMixCtrl[index].createCoefficients(sampleRate / 100, sampleRate);
        
        mFeedbackCtrl.push_back(ParameterSmooth());
        mFeedbackCtrl[index].createCoefficients(sampleRate / 100, sampleRate);

        mCutOffCtrl.push_back(ParameterSmooth());
        mCutOffCtrl[index].createCoefficients(sampleRate / 100, sampleRate);

        mSpeedCtrl.push_back(ParameterSmooth());
        mSpeedCtrl[index].createCoefficients(sampleRate / 100, sampleRate);

        mFilter.push_back(juce::IIRFilter());
        mFilter[index].setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 1200.0f, 1.0));

        modulator.push_back(Oscillator());
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

    playHead = this->getPlayHead();
    if (playHead == nullptr)
    {

    }
    else
    {
        playHead->getCurrentPosition(currentPositionInfo);
        bpm = currentPositionInfo.bpm;
        numeratorSubDivision = currentPositionInfo.timeSigNumerator;
        denominatorSubDivision = currentPositionInfo.timeSigDenominator;
        //DBG(bpm);
        //DBG(numeratorSubDivision);
        //DBG(denominatorSubDivision);
        //DBG(bpm / 60.0 * numeratorSubDivision / denominatorSubDivision);
    }


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
            //auto timeCtrl = mTimeCtrl[channel].process(mTime->get());
            //auto feedbackCtrl = mFeedbackCtrl[channel].process(mFeedback->get());
            auto mixCtrl = mMixCtrl[channel].process(mMix->get());
            auto cutOffCtrl = mCutOffCtrl[channel].process(mCutOff->get());
            auto raw = channelData[sample];

            //channelData[sample] = mCircularBuffer[channel].process(channelData[sample], timeCtrl * getSampleRate(), feedbackCtrl, mixCtrl);
            auto systemSpeed = bpm / 60.0 * numeratorSubDivision / denominatorSubDivision;
            auto modulation = modulator[channel].process(systemSpeed * mSpeed->get(), getSampleRate(), mOscType->getIndex());

            auto test = modulation * mAmount->get() + cutOffCtrl;
            if (test <= 20)
            {
                test = 20;
            }
            else if (test >= getSampleRate() / 2)
            {
                test = getSampleRate() / 2;
            }
            mCoefficient.model = mFilterType->getIndex();
            mCoefficient.setParameter(test, getSampleRate(), mResonance->get(), 0.0, 0.0);
            mFilter[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0],
                                                                   mCoefficient.getCoefficients()[1],
                                                                   mCoefficient.getCoefficients()[2],
                                                                   mCoefficient.getCoefficients()[3],
                                                                   mCoefficient.getCoefficients()[4],
                                                                   mCoefficient.getCoefficients()[5]));
            //mFilter[channel].setCoefficients(juce::IIRCoefficients::makeLowPass(getSampleRate(), test, 1.0));
            channelData[sample] = mFilter[channel].processSingleSampleRaw(channelData[sample]) * mixCtrl + raw * (1 - mixCtrl);
            //(NumericType b0, NumericType b1, NumericType b2, NumericType a0, NumericType a1, NumericType a2)
            //juce::IIRCoefficients::coefficients()
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
