/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "tables.h"
#include "myheader.h"

//==============================================================================
/**
*/
class OptobetaAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    OptobetaAudioProcessor();
    ~OptobetaAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::atomic<float>* afp_pre, * afp_threshold, * afp_gain, * afp_maxratio, * afp_dry, * afp_drive, * afp_mono,*afp_satbypass;
    ParamMapping* pm_pre, * pm_thre, * pm_gain, * pm_maxratio, * pm_dry, * pm_drive, * pm_drivepost;

    juce::AudioProcessorValueTreeState *parameters;

    float fsr;
    float lastgain;
    float ratio_meter_value;
    //bool on_process_block_repaint;

    AudioProcessorEditor* Editor;
    Lookup* lut2;
    HPF * hp0, * hp1, * hp2, * hp3;
    LDR* ldr;
    QSat* qsat0, * qsat1;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptobetaAudioProcessor)
};
