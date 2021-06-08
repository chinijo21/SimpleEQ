/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
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

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //get the filter ready
    juce::dsp::ProcessSpec sets;
    sets.maximumBlockSize = samplesPerBlock;
    sets.numChannels = 1;
    sets.sampleRate = sampleRate;

    leftChain.prepare(sets);
    rightChain.prepare(sets);



}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

   
        //requieres a proccesor context, we need an audiBlock instnce
        // extract left and right channel from buffer (0,1)
        // ..do something to the data...
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    //Create a context for both sides
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    //return new SimpleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
//Calls Struct and gets parameters
ChainSets getChainSets(juce::AudioProcessorValueTreeState& audioPro) {
    ChainSets settings;
    //If we do audioPro.getParameter("LowCut Freq")->getValue() we'll get a normalized value and thats grosso
    //So we use getRawParameterValue like:
    settings.lowFreq = audioPro.getRawParameterValue("LowCut Freq")->load();
    settings.peakFreq = audioPro.getRawParameterValue("Peak Freq")->load();
    settings.highFreq = audioPro.getRawParameterValue("HighCut Freq")->load();
    settings.peakGain = audioPro.getRawParameterValue("Peak Gain")->load();
    settings.peakQ = audioPro.getRawParameterValue("Peak Q")->load();
    settings.lowSlope = audioPro.getRawParameterValue("Low Slope")->load();
    settings.highSlope = audioPro.getRawParameterValue("High Slope")->load();

    return settings;

}

juce::AudioProcessorValueTreeState::ParameterLayout
SimpleEQAudioProcessor::createParameterLayout(){
    //Low Cut, HighCut and Bell
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    //Hearing range 20hz-20khz
    //LowCut                                                                    //fmin, fmax, incrementos, skew(response), start pos
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", 
                                                           "LowCut Freq",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20.f));
    //HighCut is the same as Low but we start 20kHz
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20000.f));
    //Peak cut we have a freq cut and dB gain/loss and the Q
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                           "Peak Gain",
                                                            juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Q",
                                                           "Peak Q",
                                                            juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));

    //TODO:Bands for eq Cut in dB/octv
    //So we hve 8 options up to 96 dB/oct (brickwall) 
    juce::StringArray bandCuts;
    for (int i = 0; i < 8; i++) {
        juce::String name;
        name << (12 + i * 12);
        name << "dB/oct";
        bandCuts.add(name);
    }

    //Add choices for LowCut
    layout.add(std::make_unique<juce::AudioParameterChoice>("Low Slope", "Low Slope", bandCuts, 0));
    //Add choices for highCut
    layout.add(std::make_unique<juce::AudioParameterChoice>("High Slope", "High Slope", bandCuts, 0));

    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
