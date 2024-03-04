/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SatCompAudioProcessor::SatCompAudioProcessor()
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
}

SatCompAudioProcessor::~SatCompAudioProcessor()
{
}

//==============================================================================
const juce::String SatCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SatCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SatCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SatCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SatCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SatCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SatCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SatCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SatCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void SatCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SatCompAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
 
    double oversampleRate = sampleRate * 4.0;
    
    oversampler.numChannels = getTotalNumInputChannels();
    oversampler.initProcessing(samplesPerBlock);
    oversampler.reset();
    
    saturator.prepareToPlay(oversampleRate);
    compressor.prepareToPlay(sampleRate);

    setLatencySamples(oversampler.getLatencyInSamples());
    
}

void SatCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SatCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void SatCompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    saturator.setOutputTrim(0.f);
    saturator.setDriveAmount(1.5f);
    
    compressor.setThreshold(-26.f);
    compressor.setMakeupGain(17.f);
    compressor.setRatio(4);
    compressor.setAttack(.450f);
    compressor.setRelease(64.f);
    
    orderSwap = false;
    
    juce::dsp::AudioBlock<float> block(buffer);
    auto N = buffer.getNumSamples();
    auto N_oversample = buffer.getNumSamples() * 4.0;
    
    
    if (!orderSwap) {
        
        auto oversampledBuffer = oversampler.processSamplesUp(block);
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {

            auto* channelData = oversampledBuffer.getChannelPointer(channel);
            saturator.process(channelData, N_oversample, channel);
        }
        
        oversampler.processSamplesDown(block);
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            
            auto* channelData = buffer.getWritePointer (channel);
            compressor.process(channelData, N, channel);
        }
    }
    else {
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            
            auto* channelData = buffer.getWritePointer (channel);
            compressor.process(channelData, N, channel);
        }
       
        auto oversampledBuffer = oversampler.processSamplesUp(block);
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {

            auto* channelData = oversampledBuffer.getChannelPointer(channel);
            saturator.process(channelData, N_oversample, channel);
        }
        
        oversampler.processSamplesDown(block);
    }
}

//==============================================================================
bool SatCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SatCompAudioProcessor::createEditor()
{
    return new SatCompAudioProcessorEditor (*this);
}

//==============================================================================
void SatCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SatCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SatCompAudioProcessor();
}
