/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OptobetaAudioProcessor::OptobetaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

    DS4x_FIR::Init(fir_h);

    hp0 = new HPF();
    hp1 = new HPF();
    hp2 = new HPF();
    hp3 = new HPF();
    ldr = new LDR();

    lut2 = new Lookup(TANHSIZE, Ttanh, 0.0f, TableMax);

    qsat0 = new QSat(fir_h);
    qsat1 = new QSat(fir_h);
    lastgain = 1.0f;
    ratio_meter_value = 0.0f;
    //on_process_block_repaint = false;

    //addParameter(freq = new AudioParameterFloat("freq","Freq",0.0f,1.0f,0.5f));
    parameters = new AudioProcessorValueTreeState(*this, nullptr, juce::Identifier("PARAMETERS"), {
        std::make_unique<juce::AudioParameterFloat>("pre", "Pre", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", -30.0f, -5.0f, -15.0f),
        std::make_unique<juce::AudioParameterFloat>("max_ratio", "Max ratio", 1.0f, 16.0f, 16.0f),
        std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 12.0f, 7.0f),
        std::make_unique<juce::AudioParameterFloat>("dry", "DRY", 0.0f, 100.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("drive", "Drive", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterBool>("mono", "MONO", false),
        std::make_unique<juce::AudioParameterBool>("sat-off", "Sat Off", false)
        });

    afp_pre = parameters->getRawParameterValue("pre");
    afp_threshold = parameters->getRawParameterValue("threshold");
    afp_maxratio = parameters->getRawParameterValue("max_ratio");
    afp_gain = parameters->getRawParameterValue("gain");
    afp_dry = parameters->getRawParameterValue("dry");
    afp_drive = parameters->getRawParameterValue("drive");
    afp_mono = parameters->getRawParameterValue("mono");
    afp_satbypass = parameters->getRawParameterValue("sat-off");



    pm_pre = new ParamMapping(0.0f);
    pm_thre = new ParamMapping(-10.0f);
    pm_gain = new ParamMapping(0.0f);
    pm_maxratio = new ParamMapping(-10.0f);
    pm_dry = new ParamMapping(0.0f);
    pm_drive = new ParamMapping(0.0f);
    pm_drivepost = new ParamMapping(0.0f);

    //pre thre gain maxratio dry drive
}

OptobetaAudioProcessor::~OptobetaAudioProcessor()
{
}

//==============================================================================
const String OptobetaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OptobetaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OptobetaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OptobetaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OptobetaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OptobetaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OptobetaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OptobetaAudioProcessor::setCurrentProgram (int index)
{
}

const String OptobetaAudioProcessor::getProgramName (int index)
{
    return {};
}

void OptobetaAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void OptobetaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    fsr = sampleRate;
}

void OptobetaAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OptobetaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void OptobetaAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
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
    auto* channelData0 = buffer.getWritePointer(0);
    auto* channelData1 = buffer.getWritePointer(1);
    float t;
    //pre thre gain maxratio dry drive
    if(pm_pre->setx(*afp_pre)){// if the convertion is needed ,do it
        pm_pre->y = powf(2.0f, pm_pre->getx() / 6.0f);
    }
    if(pm_thre->setx(*afp_threshold)){
        pm_thre->y = 1.25f / powf(2.0f, pm_thre->getx() / 6.0f);
    }
    if(pm_maxratio->setx(*afp_maxratio)){
        t = pm_maxratio->getx();
        float t_2, t_3, t_4;
        t_2 = t * t; t_3 = t_2 * t; t_4 = t_3 * t;
        pm_maxratio->y = 1.9530e-5f * t_4 - 9.4798e-4f * t_3 + 1.9689e-2f * t_2 - 2.2756e-1f * t + 1.2103f;
    }
    if (pm_drive->setx(*afp_drive)) {
        pm_drive->y = powf(2.0f, pm_drive->getx() / 6.0f) * 1.16f;
    }
    if (pm_drivepost->setx(*afp_drive)) {
        t = pm_drivepost->getx();
        t = t< 0.0f ? -t * 0.75f : -t * 0.25f;
        pm_drivepost->y = powf(2.0f, t / 6.0f);
    }
    if (pm_gain->setx(*afp_gain)){
        pm_gain->y = powf(2.0f, pm_gain->getx() / 6.0f);
    }
    if (pm_dry->setx(*afp_dry)) {
        pm_dry->y = pm_dry->getx() * 0.01;
    }

    float ratio_average=0.0f;

    float drive_hp = pm_drive->getx() > 0.0f ? pm_drive->getx() * 5.0f : 0.0f;//when 'drive' go higher , a little highpass is needed

    hp0->SetK(10 + drive_hp, fsr);
    hp1->SetK(10 + drive_hp, fsr);//dc filtering
    hp3->SetK(50, fsr);
    ldr->SetK(fsr, pm_maxratio->y);
    qsat0->SetK(fsr);
    qsat1->SetK(fsr);
    bool ismono = *afp_mono > 0.5f;
    bool satbypass = *afp_satbypass > 0.5f;
    float tg ,t2,de;
    //int sig = Debug->getIndex();
    float f1,f2,f3,f4,f5;
    f1 = pm_pre->y * pm_gain->y * (1.0f - pm_dry->y);
    f2 = pm_drive->y * 1.12f;
    f5 = pm_drivepost->y;
    int SamplesNum = buffer.getNumSamples();
    if (ismono)
    {
        f3 = pm_pre->y;
        //f3 = 0.5f * pm_pre->y;
        if (satbypass)
        {
            for (int j = 0; j < SamplesNum; j++)
            {
                t2 = (channelData0[j]) * f3;
                //t2 = (channelData0[j] + channelData1[j]) * f3;
                t2 = hp3->Run(t2);
                t2 = t2 * lastgain * pm_thre->y;//range 1-40
                t2 = ldr->Run(t2);
                lastgain = t2;//control feedback
                ratio_average += lastgain;
                tg = lastgain * f1 + pm_dry->y;

                channelData0[j] = tg * channelData0[j];
                channelData1[j] = channelData0[j];
            }
        }
        else
        {
            for (int j = 0; j < SamplesNum; j++)
            {
                t2 = (channelData0[j]) * f3;
                //t2 = (channelData0[j] + channelData1[j]) * f3;
                t2 = hp3->Run(t2);
                t2 = t2 * lastgain * pm_thre->y;//range 1-40
                t2 = ldr->Run(t2);
                lastgain = t2;//control feedback
                ratio_average += lastgain;
                tg = lastgain * f1 + pm_dry->y;
                f4 = tg * f2;

                channelData0[j] = f5 * hp0->Run(qsat0->Run2(lut2, f4 * channelData0[j]));
                channelData1[j] = channelData0[j];
            }
        }
    }
    else
    {
        f3 = 0.5f * pm_pre->y;
        if (satbypass)
        {
            for (int j = 0; j < SamplesNum; j++)
            {
                t2 = (channelData0[j] + channelData1[j]) * f3;
                t2 = hp3->Run(t2);
                t2 = t2 * lastgain * pm_thre->y;//range 1-40
                t2 = ldr->Run(t2);
                lastgain = t2;//control feedback
                ratio_average += lastgain;
                tg = lastgain * f1 + pm_dry->y;
                channelData0[j] = tg * channelData0[j];
                channelData1[j] = tg * channelData1[j];
            }
        }
        else
        {
            for (int j = 0; j < SamplesNum; j++)
            {
                t2 = (channelData0[j] + channelData1[j]) * f3;
                t2 = hp3->Run(t2);
                t2 = t2 * lastgain * pm_thre->y;//range 1-40
                t2 = ldr->Run(t2);
                lastgain = t2;//control feedback
                ratio_average += lastgain;
                tg = lastgain * f1 + pm_dry->y;
                f4 = tg * f2;
                channelData0[j] = f5 * hp0->Run(qsat0->Run2(lut2, f4 * channelData0[j]));
                channelData1[j] = f5 * hp1->Run(qsat1->Run2(lut2, f4 * channelData1[j]));
            }
        }
    }
#define METER_ALPHA_R 0.03f//higher means faster
#define METER_ALPHA_A 0.2f
    ratio_average /= buffer.getNumSamples();
    if (ratio_average < ratio_meter_value) {
        ratio_meter_value = METER_ALPHA_A * ratio_average + ratio_meter_value * (1.0f - METER_ALPHA_A);
    }
    else
    {
        ratio_meter_value = METER_ALPHA_R * ratio_average + ratio_meter_value * (1.0f - METER_ALPHA_R);
    }
    Editor = this->getActiveEditor();
    if (Editor != NULL)
    {
        Editor->repaint();
    }
}

//==============================================================================
bool OptobetaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* OptobetaAudioProcessor::createEditor()
{
    return new OptobetaAudioProcessorEditor (*this);
}

//==============================================================================
void OptobetaAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    //pre thre gain maxratio dry drive
    auto state = parameters->copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OptobetaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters->state.getType()))
            parameters->replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OptobetaAudioProcessor();
}
