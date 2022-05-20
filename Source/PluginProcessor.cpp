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
    addParameter    (mPitch      = new juce::AudioParameterInt      ("0x00",    "Pitch",      -12,    12,     0));
    addParameter    (mPreDelay   = new juce::AudioParameterFloat    ("0x02",    "Pre-Delay",  0.00f,  300.0f, 270.0f));
    addParameter    (mColor      = new juce::AudioParameterFloat    ("0x03",    "Brightness", 150,    5000,   1000));
    addParameter    (mSpread     = new juce::AudioParameterFloat    ("0x04",    "Spread",     0.00f,  1.00f,  0.50f));
    addParameter    (mFeedback   = new juce::AudioParameterFloat    ("0x05",    "Feedback",   0.00f,  1.00f,  0.150f));
    addParameter    (mDecay      = new juce::AudioParameterFloat    ("0x06",    "Decay",      0.01f,  1.00f,  0.50f));
    addParameter    (mSpeed      = new juce::AudioParameterFloat    ("0x07",    "Speed",      0.1f,   4.00f,  1.00f));
    addParameter    (mDepth      = new juce::AudioParameterFloat    ("0x08",    "Depth",      0.0f,   100.0f, 40.0f));
    addParameter    (mMix        = new juce::AudioParameterFloat    ("0x01",    "Mixing",     0.00f,  1.00f,  0.50f));
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
    Pitch_CB_1.reset(new DelayFeedback<float>[getTotalNumInputChannels()]);
    Picth_CB_2.reset(new DelayFeedback<float>[getTotalNumInputChannels()]);
    All_Pass_Delay.reset(new DelayAPF<float>[getTotalNumInputChannels()]);
    Pre_Delay.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_1.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_2.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_3.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);
    CB_4.reset(new CircularBuffer<float>[getTotalNumInputChannels()]);

    mCoefficient.model = 4;

    for (int index = 0; index < getTotalNumInputChannels(); index++)
    {
        Pitch_CB_1[index].digitalDelayLine.createCircularBuffer(sampleRate * 0.05);
        Pitch_CB_1[index].digitalDelayLine.flushBuffer();
        
        Picth_CB_2[index].digitalDelayLine.createCircularBuffer(sampleRate * 0.05);
        Picth_CB_2[index].digitalDelayLine.flushBuffer();
        
        All_Pass_Delay[index].digitalDelayLine.createCircularBuffer(8192);
        All_Pass_Delay[index].digitalDelayLine.flushBuffer();

        Pre_Delay[index].createCircularBuffer(16384);
        Pre_Delay[index].flushBuffer();
        
        CB_1[index].createCircularBuffer(4096);
        CB_1[index].flushBuffer();

        CB_2[index].createCircularBuffer(4096);
        CB_2[index].flushBuffer();

        CB_3[index].createCircularBuffer(8192);
        CB_3[index].flushBuffer();

        CB_4[index].createCircularBuffer(8192);
        CB_4[index].flushBuffer();
        
        mPitchCtrl.push_back(ParameterSmooth());
        mPitchCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);
        
        mMixCtrl.push_back(ParameterSmooth());
        mMixCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mPreDelayCtrl.push_back(ParameterSmooth());
        mPreDelayCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mSpreadCtrl.push_back(ParameterSmooth());
        mSpreadCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mColorCtrl.push_back(ParameterSmooth());
        mColorCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mFeedbackCtrl.push_back(ParameterSmooth());
        mFeedbackCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mDecayCtrl.push_back(ParameterSmooth());
        mDecayCtrl[index].createCoefficients(sampleRate * 0.001, sampleRate);

        mDepthCtrl.push_back(ParameterSmooth());
        mDepthCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mSpeedCtrl.push_back(ParameterSmooth());
        mSpeedCtrl[index].createCoefficients(sampleRate * 0.0001, sampleRate);

        mFilter_1.push_back(juce::IIRFilter());
        mFilter_2.push_back(juce::IIRFilter());
        mHighPass.push_back(juce::IIRFilter());
        
        feedbackLoop_1.push_back(0.0f);
        feedbackLoop_2.push_back(0.0f);
        feedbackLoop_3.push_back(0.0f);
        feedbackLoop_4.push_back(0.0f);

        modulator_1.push_back(Oscillator());
        modulator_2.push_back(Oscillator());
        modulator_3.push_back(Oscillator());
        modulator_4.push_back(Oscillator());
        pitch_modulator_1.push_back(Oscillator());
        pitch_modulator_2.push_back(Oscillator());
        spread_osc.push_back(Oscillator());
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
        
        auto colorCtrl = mColorCtrl[channel].process(mColor->get());
        mCoefficient.setParameter(colorCtrl, getSampleRate(), 0, 0, 0);
        mFilter_1[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0], 0, 0, mCoefficient.getCoefficients()[3], mCoefficient.getCoefficients()[4], 0));
        mFilter_2[channel].setCoefficients(juce::IIRCoefficients(mCoefficient.getCoefficients()[0], 0, 0, mCoefficient.getCoefficients()[3], mCoefficient.getCoefficients()[4], 0));

        mHighPass[channel].setCoefficients(juce::IIRCoefficients(0.994975f, -1.989950f, 0.994975f, 1.0, -1.989925f, 0.989976f));
        
        auto rad_fixed = 0.125 * TWO_PI;

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // ..do something to the data...
            auto drySignal = channelData[sample];

            // ramping process 
            auto preDelayCtrl = mPreDelayCtrl[channel].process(mPreDelay->get()) * 0.001f;
            auto feedbackCtrl = mFeedbackCtrl[channel].process(mFeedback->get());
            auto pitchCtrl = ((powf(2, (mPitchCtrl[channel].process(mPitch->get()) / 12.0f))) - 1.0f) * -20.0f;
            auto mixCtrl = mMixCtrl[channel].process(mMix->get());
            
            auto decayCtrl = mDecayCtrl[channel].process(mDecay->get()) * 0.5f + 0.5f;
            auto depthCtrl = mDepthCtrl[channel].process(mDepth->get());
            
//            auto speedCtrl = mSpeedCtrl[channel].process(mSpeed->get());
//            auto spreadCtrl = mSpreadCtrl[channel].process(mSpread->get());
            
            // pitch shifter
            auto phasor_1 = pitch_modulator_1[channel].process(pitchCtrl, getSampleRate(), 5, 0.0f);
            auto phasor_2 = pitch_modulator_2[channel].process(pitchCtrl, getSampleRate(), 6, 0.0f);
            auto window_1 = ((cosf(TWO_PI * phasor_1) * -1.0f) + 1.0f) / 2.0f;
            auto window_2 = ((cosf(TWO_PI * phasor_2) * -1.0f) + 1.0f) / 2.0f;
            auto delay_1 = Pitch_CB_1[channel].process(drySignal, phasor_1 * getSampleRate() * 0.05f, 0, 1.0f) * window_1;
            auto delay_2 = Picth_CB_2[channel].process(drySignal, phasor_2 * getSampleRate() * 0.05f, 0, 1.0f) * window_2;
            auto pitched_output = (delay_1 + delay_2) * 0.25f;
            
            // early reflection
            auto temp = Pre_Delay[channel].readBuffer(preDelayCtrl * getSampleRate() + 1, true);
            auto t3 = All_Pass_Delay[channel].processSchroeder(temp, 7001, 0.75f);
            auto t2 = t3 * feedbackCtrl;
            auto t1 = mHighPass[channel].processSingleSampleRaw(pitched_output + t2);
            Pre_Delay[channel].writeBuffer(t1);
            
            // room reverberation
            auto modulation_1 = modulator_1[channel].process(1, getSampleRate(), 0, 0.0f);
            auto modulation_2 = modulator_2[channel].process(1.005f, getSampleRate(), 0, 0.25f * TWO_PI);
            auto modulation_3 = modulator_3[channel].process(1.005f * 1.005f, getSampleRate(), 0, 0.50f * TWO_PI);
            auto modulation_4 = modulator_4[channel].process(1.005f * 1.005f * 1.005f, getSampleRate(), 0, 0.75f * TWO_PI);

            feedbackLoop_1[channel] = CB_1[channel].readBuffer((1942.0f + modulation_1 * depthCtrl), true);
            feedbackLoop_2[channel] = CB_2[channel].readBuffer((2880.0f + modulation_2 * depthCtrl), true);
            feedbackLoop_3[channel] = CB_3[channel].readBuffer((4224.0f + modulation_3 * depthCtrl), true);
            feedbackLoop_4[channel] = CB_4[channel].readBuffer((6480.0f + modulation_4 * depthCtrl), true);

            auto temp1 = feedbackLoop_1[channel] * cosf(rad_fixed) - feedbackLoop_2[channel] * sinf(rad_fixed);
            auto temp2 = feedbackLoop_1[channel] * sinf(rad_fixed) + feedbackLoop_2[channel] * cosf(rad_fixed);
            auto temp3 = feedbackLoop_3[channel] * cosf(rad_fixed) - feedbackLoop_4[channel] * sinf(rad_fixed);
            auto temp4 = feedbackLoop_3[channel] * sinf(rad_fixed) + feedbackLoop_4[channel] * cosf(rad_fixed);

            auto output_1 = temp1 * cosf(rad_fixed) - temp3 * sinf(rad_fixed);
            auto output_2 = temp1 * sinf(rad_fixed) + temp3 * cosf(rad_fixed);
            auto output_3 = temp2 * cosf(rad_fixed) - temp4 * sinf(rad_fixed);
            auto output_4 = temp2 * sinf(rad_fixed) + temp4 * cosf(rad_fixed);

            CB_1[channel].writeBuffer(t1 + output_1 * decayCtrl);
            CB_2[channel].writeBuffer(t2 + output_2 * decayCtrl);
            CB_3[channel].writeBuffer(t3 + mFilter_1[channel].processSingleSampleRaw(output_3) * decayCtrl);
            CB_4[channel].writeBuffer(drySignal + mFilter_2[channel].processSingleSampleRaw(output_4) * decayCtrl);

            channelData[sample] = output_1 * mixCtrl + drySignal * (1 - mixCtrl);
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
