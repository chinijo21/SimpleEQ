/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

//**Data structure representing all the parameters values
struct ChainSets {
    int lowSlope{ 0 };
    int highSlope{ 0 };
    float lowFreq{ 0 };
    float highFreq{ 0 };
    float peakFreq{ 0 };
    float peakGain{ 0 };
    float peakQ{ 0 };
};
//Helper function for giving us all the parameters in the struct above
ChainSets getChainSets(juce::AudioProcessorValueTreeState& audioPro);

class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

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
    //AudioProcessor Name{who?, undo?, Parameters, Parameters Layout} we need to give him 
    //all the parameters
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState audioPro{ *this, nullptr, "Parameters", createParameterLayout()};

private:
    //Sets aliases for filters
    using Filter = juce::dsp::IIR::Filter<float>;
    using cutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter>;

    //highPass, LowPass, Peak, Shelf, Notch
    // Mono Chain: low -> parametric -> highCut x2 to have stereo
    using monoChain = juce::dsp::ProcessorChain<cutFilter, Filter, cutFilter>;
    monoChain leftChain, rightChain;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
 