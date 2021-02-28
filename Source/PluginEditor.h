/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "atlbase.h"
#include "atlstr.h"

//==============================================================================
/**
*/

#define DEG2RAD 0.01745329252

class mSlider : public Slider
{
private:
    String name,unit;
    int x, y, w, h;
    Component* father;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attach;
public:
    mSlider(String Sname,String Unit, int tx, int ty, int tw, int th,float ParaLB,float ParaUB, Component* tfather, Slider::Listener* listener)
    {
        x = tx;
        y = ty;
        w = tw;
        h = th;
        name = Sname;
        setName(Sname);
        unit = Unit;
        //setSliderStyle(Slider::LinearVertical);
        setSliderStyle(Slider::Rotary);
        //setRange(ParaLB, ParaUB, (ParaUB - ParaLB)/1000.0f);
        setTextBoxStyle(Slider::NoTextBox, false, w-10, 30);
        setPopupDisplayEnabled(true, false, tfather);
        setTextValueSuffix(unit);
        father = tfather;
        tfather->addAndMakeVisible(this);
        addListener(listener);
    }
    void attach_reset(AudioProcessorValueTreeState::SliderAttachment* _attach)
    {
        attach.reset(_attach);
    }
    Slider* get_slider_pointer()
    {
        return (Slider*)this;
    }
    void parentDraw(Graphics& g)
    {
        int LabelW = 60;
        int LabelH = 16;
        int cx = w / 2 + x;
        int cy = h / 2 + y;
        int cr = (int)(min(w, h) * 0.8f * 0.5f + 0.5f);
        int ty = (int)(cy * 1.0f + cr * 0.866f + (h * 0.5f - cr * 0.866f) * 0.5f + 0.5f);//0.866 from sqrt(3)/2
        g.setColour(Colour::fromRGB(16,148,156));
        Font tf = g.getCurrentFont();
        tf.setBold(false);
        g.setFont(tf);
        g.setFont(13.0);
        g.setColour(Colour::fromRGB(38, 46, 46));
        g.fillRect(cx - LabelW / 2, ty - LabelH / 2 -2 , LabelW, LabelH);
        g.drawSingleLineText(name, cx + 1, ty+3, Justification::horizontallyCentred);
        g.setColour(Colour::fromRGB(16, 255, 255));
        g.drawSingleLineText(name, cx, ty+2, Justification::horizontallyCentred);
    }
    void onResized()
    {
        setBounds(x, y, w, h);
    }
    void onValueChanged()
    {
        ;//*(value) = getValue(); 
    }
};

class OtherLookAndFeel : public LookAndFeel_V4
{
public:
    bool sat_bypass;
    OtherLookAndFeel()
    {
        //setColour(Slider::thumbColourId, Colours::grey);
        //setColour(Slider::backgroundColourId, Colours::whitesmoke);
        //setColour(Slider::trackColourId, Colours::lightgrey);
        sat_bypass = false;
    }
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& s) override
    {
        int cx = x + width / 2;
        int cy = sliderPos;
        int trackW = 5;
        int thumbW = 50;
        int thumbH = 20;
        int thumbLineW = 26;
        g.setColour(Colour::fromRGB(5,17,18));//#1
        g.fillRoundedRectangle(cx - trackW / 2 - 2, y - 2, trackW, height + 1, 2);
        g.setColour(Colour::fromRGB(22, 47, 48));//#2
        g.fillRoundedRectangle(cx - trackW / 2, y, trackW, height , 2);
        g.setColour(Colour::fromRGB(0, 240, 252));//#3
        g.fillRoundedRectangle(cx - trackW / 2, sliderPos, trackW, height - sliderPos + y, 2);
        g.setColour(Colour::fromRGB(30, 30, 30));//#4
        g.fillRoundedRectangle(cx - thumbW / 2 + 3, cy - thumbH / 2 + 3, thumbW, thumbH, 2);
        g.setColour(Colour::fromRGB(94, 94, 94));//#5
        g.fillRoundedRectangle(cx - thumbW / 2, cy - thumbH / 2, thumbW, thumbH, 2);
        g.setColour(Colour::fromRGB(0, 242, 255));//#6
        g.fillRoundedRectangle(cx - thumbLineW/2, cy - 1, thumbLineW, 2, 2);
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& s) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2)*0.8f;//r
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        juce::Path p;
        
        p.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.618f);
        g.setColour(Colour::fromRGB(22, 47, 48));
        g.fillPath(p);

        p.clear();
        p.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.618f);
        g.setColour(Colour::fromRGB(22, 178, 186));
        g.fillPath(p);

        auto pointerLength = radius * 0.382f;
        auto pointerThickness = 5.0f;
        p.clear();
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(Colour::fromRGB(0, 242, 255));
        g.fillPath(p);

        Font tf = g.getCurrentFont();
        auto text = s.getTextFromValue(s.getValue());

        if (sat_bypass == true && s.getName()=="Drive")
        {
            text = String("Off");
        }

        tf.setBold(false);
        g.setFont(tf);
        g.setFont(12.0);
        g.setColour(Colour::fromRGB(26, 166, 173));
        g.drawSingleLineText(text, centreX, centreY+2.0f, Justification::horizontallyCentred);
        
    }
};


class OptobetaAudioProcessorEditor  : public AudioProcessorEditor,
                                      public Slider::Listener,
                                      public Button::Listener
{
public:
    OptobetaAudioProcessorEditor (OptobetaAudioProcessor&);
    ~OptobetaAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void Draw_Meter(Graphics&);
    void Draw_BG(Graphics&);
    void Draw_Panel_Line(int x0,int y0,int x1,int y1,Graphics&,int type);
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    void sliderValueChanged(Slider* slider) override;
    void buttonStateChanged(Button* button) override;
    void buttonClicked(Button* button) override;
    OptobetaAudioProcessor& processor;

    OtherLookAndFeel otherLookAndFeel;
    //* PRE, * THRESHOLD, * GAIN ,*MAXRATIO ,*DRY,*DRIVE;
    mSlider* slider_pre, * slider_thre, * slider_ratio, * slider_gain, *slider_dry, * slider_drive;

    TextButton* button_mono,*button_satbypass;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> monoAttachment,satbypassAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptobetaAudioProcessorEditor)
};
