/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OptobetaAudioProcessorEditor::OptobetaAudioProcessorEditor (OptobetaAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    int winW = 625, winH = 500, SW = 75,SLeft = 50,Sy0 = 125,SH = 300;
    int rx0 = 50, rx1 = 163, rx2 = 275, rx3 = 387, rx4 = 500,ry0 = 100,ry1 =240;
    int w0 = 112, h0 = 140, w1 = 225, h1 = 210;
    setLookAndFeel(&otherLookAndFeel);
    
    slider_pre = new mSlider(String("Pre"), String("db"), rx0+1,ry0 ,w0,h0, -12.0f, 12.0f, this, this);
    slider_thre = new mSlider(String("Threshold"), String("db"), rx1, ry0, w0, h0, -30.0f, -5.0f, this, this);
    slider_ratio = new mSlider(String("Ratio"), String(""), rx2, ry0, w0, h0, 1.0f, 16.0f, this, this);
    slider_gain = new mSlider(String("Gain"), String("db"), rx3, ry0, w0, h0, 0.0f, 12.0f, this, this);

    slider_dry = new mSlider(String("Dry"), String("%"), rx0, ry1, w1, h1, 0.0f, 100.0f, this, this);
    slider_drive = new mSlider(String("Drive"), String("db"), rx2, ry1, w1, h1, -12.0f, 12.0f, this, this);

    slider_pre->attach_reset(new AudioProcessorValueTreeState::SliderAttachment(*processor.parameters, "pre", *slider_pre->get_slider_pointer()));
    slider_thre->attach_reset(new AudioProcessorValueTreeState::SliderAttachment(*processor.parameters, "threshold", *slider_thre->get_slider_pointer()));
    slider_ratio->attach_reset(new AudioProcessorValueTreeState::SliderAttachment(*processor.parameters, "max_ratio", *slider_ratio->get_slider_pointer()));
    slider_gain->attach_reset(new AudioProcessorValueTreeState::SliderAttachment(*processor.parameters, "gain", *slider_gain->get_slider_pointer()));
    slider_dry->attach_reset(new AudioProcessorValueTreeState::SliderAttachment(*processor.parameters, "dry", *slider_dry->get_slider_pointer()));
    slider_drive->attach_reset(new AudioProcessorValueTreeState::SliderAttachment(*processor.parameters, "drive", *slider_drive->get_slider_pointer()));
    
    button_mono = new TextButton();
    button_mono->setBounds(10,10,60,30);
    button_mono->setButtonText("Mono");
    monoAttachment.reset(new AudioProcessorValueTreeState::ButtonAttachment(*processor.parameters, "mono", *button_mono));
    button_mono->setColour(juce::Label::backgroundColourId, Colour::fromRGB(30, 30, 30));
    button_mono->setColour(juce::TextButton::textColourOnId ,Colour::fromRGB(0, 242 ,255));
    button_mono->setColour(juce::TextButton::textColourOffId, Colour::fromRGB(0, 50, 0));
    this->addAndMakeVisible(button_mono);
    button_mono->setClickingTogglesState(true);
    button_mono->addListener(this);

    button_satbypass = new TextButton();
    button_satbypass->setBounds(80,10,60,30);
    button_satbypass->setButtonText("Sat Off");
    satbypassAttach.reset(new AudioProcessorValueTreeState::ButtonAttachment(*processor.parameters, "sat-off", *button_satbypass));
    button_satbypass->setColour(juce::Label::backgroundColourId, Colour::fromRGB(30, 30, 30));
    button_satbypass->setColour(juce::TextButton::textColourOnId, Colour::fromRGB(0, 242, 255));
    button_satbypass->setColour(juce::TextButton::textColourOffId, Colour::fromRGB(0, 50, 0));
    this->addAndMakeVisible(button_satbypass);
    button_satbypass->setClickingTogglesState(true);
    button_satbypass->addListener(this);

    setSize (winW, winH);
}

OptobetaAudioProcessorEditor::~OptobetaAudioProcessorEditor()
{
    delete slider_pre;
    delete slider_ratio;
    delete slider_thre;
    delete slider_gain;
    delete slider_dry;
    delete slider_drive;
    setLookAndFeel(nullptr);
}

void OptobetaAudioProcessorEditor::Draw_Panel_Line(int x0,int y0,int x1,int y1,Graphics& g,int type)
{
    g.setColour(Colour::fromRGB(65, 65, 65));
    g.drawLine(x0, y0, x1, y1, type==0 ? 4.0f : 5.0f);
    g.setColour(Colour::fromRGB(36, 36, 36));
    g.drawLine(x0, y0, x1, y1, type==0? 1.0f : 2.0f);
}

void OptobetaAudioProcessorEditor::Draw_BG(Graphics& g)
{
    g.fillAll(Colour::fromRGB(56,56,56));
    Draw_Panel_Line(50, 100, 575, 100, g, 1);
    Draw_Panel_Line(50, 450, 575, 450, g, 1);
    Draw_Panel_Line(50, 100, 50, 450, g, 1);
    Draw_Panel_Line(500, 100, 500, 450, g, 1);
    Draw_Panel_Line(575, 100, 575, 450, g, 1);
    Draw_Panel_Line(50, 240, 500, 240, g, 1);

    g.setColour(Colour::fromRGB(27, 35, 36));
    g.setFont(30.0f);
    g.setColour(Colour::fromRGB(26, 72, 74));
    g.drawText("OptoBeta Compressor", 75+2, 25+2, 475, 50, Justification::centred);
    g.setColour(Colour::fromRGB(24,136,140));
    g.drawText("OptoBeta Compressor", 75, 25, 475, 50, Justification::centred);
}

void OptobetaAudioProcessorEditor::Draw_Meter(Graphics& g)
{
    int x, y, w, h;
    x = 525; y = 135; w = 25; h = 275;
    float t = processor.ratio_meter_value;
    t = 20.0f * log10f(t);
    t = t > 0.0f ? 0.0f : t;
    t = t < -30.0f ? -30.0f : t;
    t = (-t) / 30.0f;
    g.setColour(Colour::fromRGB(27, 35, 36));
    g.drawRect(x - 2, y - 2, w, h ,4);
    g.setColour(Colour::fromRGB(118, 122, 122));
    g.drawRect(x, y, w, h, 4);
    g.setColour(Colour::fromRGB(117,149,150));
    g.setFont(10.0f);
    for (int i = 0; i <= 30; i += 3)
    {
        int ty = (int)((i / 30.0f)* h + y+2.5f);
        g.drawSingleLineText('-' + String(i), x + w + 5, ty, juce::Justification::left);
    }
    g.setColour(Colour::fromRGB(22, 47, 48));
    g.fillRect(x, y, w, h);
    g.setColour(Colour::fromRGB(22, 178, 186));
    g.fillRect(x, y, w, (int)(h * t+0.49999f));//0~-30db
    int LabelW = 60,bias=15;
    Font tf = g.getCurrentFont();
    tf.setBold(false);
    x = 500;
    g.setFont(tf);
    g.setFont(13.0);
    g.setColour(Colour::fromRGB(38, 46, 46));
    g.fillRect(x + (37 - LabelW / 2), h + y + bias, LabelW, 15);
    x = 525;
    g.drawSingleLineText("Reduce", x + w / 2 + 1, h + y + bias + 10 + 2, Justification::horizontallyCentred);
    g.setColour(Colour::fromRGB(16, 255, 255));
    g.drawSingleLineText("Reduce", x + w / 2, h + y + bias + 10 + 1, Justification::horizontallyCentred);
}

//==============================================================================
void OptobetaAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    if (*(processor.afp_satbypass) > 0.5f)
    {
        otherLookAndFeel.sat_bypass = true;
    }
    else
    {
        otherLookAndFeel.sat_bypass = false;
    }
    Draw_BG(g);
    Draw_Meter(g);
    slider_pre->parentDraw(g);
    slider_thre->parentDraw(g);
    slider_ratio->parentDraw(g);
    slider_gain->parentDraw(g);
    slider_dry->parentDraw(g);
    slider_drive->parentDraw(g);
}

void OptobetaAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    slider_pre->onResized();
    slider_thre->onResized();
    slider_ratio->onResized();
    slider_gain->onResized();
    slider_dry->onResized();
    slider_drive->onResized();
}


void OptobetaAudioProcessorEditor::buttonStateChanged(Button* button)
{
    ;
}

void OptobetaAudioProcessorEditor::sliderValueChanged(Slider* slider) { ; };
void OptobetaAudioProcessorEditor::buttonClicked(Button* button)
{
    ;
}