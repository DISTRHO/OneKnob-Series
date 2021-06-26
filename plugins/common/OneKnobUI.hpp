/*
 * DISTRHO OneKnob Series
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#pragma once

// IDE helper (not needed for building)
#include "OpenGL.hpp"

#include "DistrhoUI.hpp"

#include "Blendish.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

#ifdef __clang__
# define MATH_CONSTEXPR
#else
# define MATH_CONSTEXPR constexpr
#endif

static inline MATH_CONSTEXPR int lin2dbint(const float value)
{
    return static_cast<int>(20.0f * std::log10(value) - 0.5f);
}

// --------------------------------------------------------------------------------------------------------------------

static const uint kSidePanelWidth = 12;

// --------------------------------------------------------------------------------------------------------------------

struct ThemeInitializer {
    ThemeInitializer()
    {
        BlendishTheme theme = blendishGetDefaultTheme();
        theme.labelTheme.textColor = Color::fromHTML("#cacacb");
        theme.checkBoxTheme.textColor = Color::fromHTML("#cacacb");
//         theme.checkBoxTheme.textSelectedColor = Color::fromHTML("#fff");
        blendishSetTheme(theme);
    }

    static ThemeInitializer& getInstance()
    {
        static ThemeInitializer themeinit;
        return themeinit;
    }

    struct SharedInstance {
        SharedInstance()
        {
            ThemeInitializer::getInstance();
        }
    };
};

// --------------------------------------------------------------------------------------------------------------------

class BlendishMeterLine : public BlendishSubWidget
{
public:
    explicit BlendishMeterLine(BlendishSubWidgetSharedContext* const parent, const Color& c)
        : BlendishSubWidget(parent),
          nvg(parent->getNanoVGInstance()),
          color(c),
          writeIndex(0)
    {
        std::memset(lines, 0, sizeof(lines));
        setSize(getMinimumWidth(), 40);
    }

    void push(const float value)
    {
        lines[writeIndex] = std::abs(value);
        if (++writeIndex == sizeof(lines)/sizeof(lines[0]))
            writeIndex = 0;
    }

protected:
    uint getMinimumWidth() const noexcept override
    {
        return sizeof(lines)/sizeof(lines[0]);
    }

    void onBlendishDisplay() override
    {
        const int size = sizeof(lines)/sizeof(lines[0]);
        const int height = getHeight();
        const int startX = getAbsoluteX();
        const int startY = getAbsoluteY() + height;

        int k = writeIndex;

        nvg.beginPath();

        nvg.moveTo(startX, startY - lines[k] * height);

        for (int i=0; i<size; ++i, ++k)
        {
            if (k == size)
                k = 0;
            nvg.lineTo(startX + i, startY - lines[k] * height);
        }

        nvg.lineTo(startX + size, startY - lines[k-1] * height);

        nvg.strokeColor(color);
        nvg.strokeWidth(1);
        nvg.stroke();
    }

private:
    NanoVG& nvg;
    Color color;

    float lines[256];
    int writeIndex;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlendishMeterLine)
};

// --------------------------------------------------------------------------------------------------------------------

class OneKnobUI : public UI,
                  public BlendishButtonGroup::Callback,
                  public BlendishComboBox::Callback,
                  public ButtonEventHandler::Callback,
                  public KnobEventHandler::Callback
{
public:
    OneKnobUI(const uint width, const uint height)
        : UI(width, height),
          tisi(),
          blendish(this),
          blendishTopLabel(&blendish),
          blendishAuxButtonGroupValues(nullptr),
          blendishAuxComboBoxValues(nullptr),
          blendishMeterInLabel(&blendish),
          blendishMeterOutLabel(&blendish),
          blendishMeterOutLine(&blendish, Color::fromHTML("#c90054")),
          blendishMeterInLine(&blendish, Color(0x3E, 0xB8, 0xBE, 0.75f))
    {
        const double scaleFactor = getScaleFactor();

        if (scaleFactor != 1.0)
            setSize(width * scaleFactor, height * scaleFactor);

        blendish.setScaleFactor(scaleFactor * 2);
        blendish.setSize(width * scaleFactor, height * scaleFactor);
        blendishTopLabel.setColor(Color(1.0, 1.0f, 1.0f));
        blendishTopLabel.setLabel(DISTRHO_PLUGIN_BRAND " " DISTRHO_PLUGIN_NAME);

        blendishMeterInLabel.setColor(Color(0x3E, 0xB8, 0xBE, 0.75f));
        blendishMeterInLabel.setLabel("In: -inf dB");
        blendishMeterInLabel.setFontSize(8);
        blendishMeterOutLabel.setColor(Color::fromHTML("#c90054"));
        blendishMeterOutLabel.setLabel("Out: -inf dB");
        blendishMeterOutLabel.setFontSize(8);
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(const uint32_t index, const float value) override
    {
        switch (index)
        {
        case kParameterCount + kOneKnobParameterLineUpdateTickIn:
            pushInputMeter(std::abs(value));
            break;
        case kParameterCount + kOneKnobParameterLineUpdateTickOut:
            pushOutputMeter(std::abs(value));
            break;
        }
    }

    void programLoaded(uint32_t) override
    {
    }

    void stateChanged(const char*, const char*) override
    {
    }

    // ----------------------------------------------------------------------------------------------------------------

    void onDisplay() override
    {
        const GraphicsContext& context(getGraphicsContext());
        const double scaleFactor = getScaleFactor();
        const uint width  = getWidth();
        const uint height = getHeight();

        // background
        Color::fromHTML("#222126").setFor(context);
        Rectangle<uint>(0, 0, width, height).draw(context);

        // frame inner
        Color::fromHTML("#3b393d").setFor(context);
        Rectangle<uint>((kSidePanelWidth + 1) * scaleFactor,
                        (kSidePanelWidth + 1) * scaleFactor,
                        width - (kSidePanelWidth + 1) * scaleFactor * 2,
                        height - (kSidePanelWidth + 1) * scaleFactor * 2).drawOutline(context, scaleFactor);

        // frame outer
        Color::fromHTML("#000000").setFor(context);
        Rectangle<uint>(kSidePanelWidth * scaleFactor,
                        kSidePanelWidth * scaleFactor,
                        width - kSidePanelWidth * scaleFactor * 2,
                        height - kSidePanelWidth * scaleFactor * 2).drawOutline(context, scaleFactor);

        // brand line
        {
            const uint y = kSidePanelWidth * scaleFactor + blendishTopLabel.getHeight() * 2 + 2 * scaleFactor;
            // for text use #cacacb
            Color::fromHTML("#3b393d").setFor(context);
            Line<uint>((kSidePanelWidth + 4) * scaleFactor, y,
                       width - (kSidePanelWidth + 4) * scaleFactor, y).draw(context);
        }

        // metering bounds
        Color::fromHTML("#31363b").setFor(context);
        Rectangle<int>(blendishMeterInLine.getAbsoluteX() * 2,
                       blendishMeterInLine.getAbsoluteY() * 2,
                       blendishMeterInLine.getWidth() * 2,
                       blendishMeterInLine.getHeight() * 2).drawOutline(context);
    }

    // ----------------------------------------------------------------------------------------------------------------

    void uiFocus(const bool focus, CrossingMode) override
    {
        if (focus)
            return;

        // fake event at (0,0) to close any open menus
        {
            MouseEvent ev;
            ev.button = 1;
            onMouse(ev);
        }

        // fake event at (0,0) to remove any hover state
        {
            MotionEvent ev;
            onMotion(ev);
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // main control

    void createMainControl(const Rectangle<uint>& area, const OneKnobMainControl& control)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishMainControl == nullptr,);

        BlendishKnob* const knob = new BlendishKnob(&blendish);

        knob->setCallback(this);
        knob->setId(control.id);
        knob->setLabel(control.label);
        knob->setRange(control.min, control.max);
        knob->setDefault(control.def);
        knob->setUnit(control.unit);

        mainControlArea = getScaledArea(area);
        blendishMainControl = knob;
    }

    void setMainControlValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishMainControl != nullptr,);

        blendishMainControl->setValue(value, false);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // aux buttongroup

    void createAuxiliaryButtonGroup(const Rectangle<uint>& area, const OneKnobAuxiliaryButtonGroup& option)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionButtonGroup == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxButtonGroupValues == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(option.count != 0,);

        BlendishButtonGroup* const buttonGroup = new BlendishButtonGroup(&blendish);
        BlendishLabel* const label = new BlendishLabel(&blendish);

        for (uint i=0; i<option.count; ++i)
            buttonGroup->addButton(option.values[i].value, option.values[i].label);

        buttonGroup->setCallback(this);
        buttonGroup->setId(option.id);

        label->setId(option.id);
        label->setLabel(option.description);
        label->setFontSize(10);

        auxOptionArea = getScaledArea(area);
        blendishAuxButtonGroupValues = option.values;
        blendishAuxOptionButtonGroup = buttonGroup;
        blendishAuxOptionLabel = label;
    }

    void setAuxiliaryButtonGroupValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionButtonGroup != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(value >= 0.0f,);

        const uint index = static_cast<uint>(value + 0.5f);

        blendishAuxOptionButtonGroup->setActiveButton(index, false);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // aux checkbox

    void createAuxiliaryCheckBox(const Rectangle<uint>& area, const OneKnobAuxiliaryCheckBox& option)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionCheckBox == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);

        BlendishCheckBox* const checkBox = new BlendishCheckBox(&blendish);
        BlendishLabel* const label = new BlendishLabel(&blendish);

        checkBox->setCallback(this);
        checkBox->setId(option.id);
        checkBox->setLabel(option.title);

        label->setId(option.id);
        label->setLabel(option.description);
        label->setFontSize(10);

        auxOptionArea = getScaledArea(area);
        blendishAuxOptionCheckBox = checkBox;
        blendishAuxOptionLabel = label;
    }

    void setAuxiliaryCheckBoxValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionCheckBox != nullptr,);

        blendishAuxOptionCheckBox->setChecked(value > 0.5f, false);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // aux combobox

    void createAuxiliaryComboBox(const Rectangle<uint>& area, const OneKnobAuxiliaryComboBox& option)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionComboBox == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxComboBoxValues == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(option.count != 0,);

        BlendishComboBox* const comboBox = new BlendishComboBox(&blendish);
        BlendishLabel* const label = new BlendishLabel(&blendish);

        for (uint i=0; i<option.count; ++i)
        {
            BlendishMenuItem* const item = comboBox->addMenuItem(option.values[i].label);
            item->setId(i);
        }

        comboBox->setCallback(this);
        comboBox->setDefaultLabel(option.title);
        comboBox->setId(option.id);

        label->setId(option.id);
        label->setLabel("Loading...");
        label->setFontSize(10);

        auxOptionArea = getScaledArea(area);
        blendishAuxComboBoxValues = option.values;
        blendishAuxOptionComboBox = comboBox;
        blendishAuxOptionLabel = label;
    }

    void setAuxiliaryComboBoxValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionComboBox != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxComboBoxValues != nullptr,);

        const int index = static_cast<int>(value + 0.5f);
        DISTRHO_SAFE_ASSERT_INT_RETURN(index >= 0 && index <= 4, index,); // FIXME max range

        blendishAuxOptionComboBox->setCurrentIndex(index, false);
        blendishAuxOptionLabel->setLabel(blendishAuxComboBoxValues[index].description, false);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // aux text

    void createAuxiliaryText(const Rectangle<uint>& area, const char* const text)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);

        BlendishLabel* const label = new BlendishLabel(&blendish);

        label->setLabel(text);
        label->setFontSize(10);

        auxOptionArea = getScaledArea(area);
        blendishAuxOptionLabel = label;
    }

    // ----------------------------------------------------------------------------------------------------------------

    void pushInputMeter(const float value)
    {
        blendishMeterInLine.push(std::min(1.1f, value));

        if (value < 0.0001f)
            return blendishMeterInLabel.setLabel("In: -inf dB");

        char strBuf[0xff];
        snprintf(strBuf, sizeof(strBuf), "In: %02d dB", lin2dbint(value));
        blendishMeterInLabel.setLabel(strBuf);
    }

    void pushOutputMeter(const float value)
    {
        blendishMeterOutLine.push(std::min(1.1f, value));

        if (value < 0.0001f)
            return blendishMeterOutLabel.setLabel("Out: -inf dB");

        char strBuf[0xff];
        snprintf(strBuf, sizeof(strBuf), "Out: %02d dB", lin2dbint(value));
        blendishMeterOutLabel.setLabel(strBuf);
    }

    void repositionWidgets()
    {
        const double scaleFactor = getScaleFactor();
        const uint width = getWidth();
        const uint height = getHeight();

        // top panel
        blendishTopLabel.setAbsoluteX(4 * scaleFactor);
        blendishTopLabel.setAbsoluteY(8 * scaleFactor);
        blendishTopLabel.setWidth(width - (kSidePanelWidth + 10) * scaleFactor);

        // main control
        if (BlendishKnob* const knob = blendishMainControl.get())
        {
            knob->setAbsoluteX(mainControlArea.getX());
            knob->setAbsoluteY(mainControlArea.getY());
            knob->setSize(mainControlArea.getSize());
        }

        // auxiliary options
        uint auxWidgetHeight = 0;
        uint auxWidgetPosX = 0;

        if (BlendishButtonGroup* const buttonGroup = blendishAuxOptionButtonGroup.get())
        {
            buttonGroup->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - buttonGroup->getWidth()/2);
            buttonGroup->setAbsoluteY(auxOptionArea.getY());
            auxWidgetHeight = buttonGroup->getHeight();
            auxWidgetPosX = buttonGroup->getAbsoluteX();
        }

        if (BlendishCheckBox* const checkBox = blendishAuxOptionCheckBox.get())
        {
            checkBox->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - checkBox->getWidth()/2);
            checkBox->setAbsoluteY(auxOptionArea.getY());
            auxWidgetHeight = checkBox->getHeight();
            auxWidgetPosX = checkBox->getAbsoluteX();
        }

        if (BlendishComboBox* const comboBox = blendishAuxOptionComboBox.get())
        {
            comboBox->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - comboBox->getWidth()/2);
            comboBox->setAbsoluteY(auxOptionArea.getY());
            auxWidgetHeight = comboBox->getHeight();
            auxWidgetPosX = comboBox->getAbsoluteX();
        }

        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
        {
            label->setAbsoluteX(auxWidgetPosX);
            label->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight + 2);
            label->setWidth(auxOptionArea.getWidth() - (auxWidgetPosX - auxOptionArea.getX()));
            label->setHeight(auxOptionArea.getHeight() - auxWidgetHeight - 2);
        }

        // metering
        blendishMeterInLine.setAbsolutePos(kSidePanelWidth * scaleFactor,
                                           height / 2 - blendishMeterInLine.getHeight() - kSidePanelWidth);
        blendishMeterOutLine.setAbsolutePos(kSidePanelWidth * scaleFactor,
                                            height / 2 - blendishMeterOutLine.getHeight() - kSidePanelWidth);

        blendishMeterInLabel.setAbsolutePos(width /2 - 60, height - 200);
        blendishMeterOutLabel.setAbsolutePos(width / 2 - 60, height - 190);
    }

private:
    ThemeInitializer::SharedInstance tisi;

    BlendishSubWidgetSharedContext blendish;

    // top panel
    BlendishLabel blendishTopLabel;

    // main knob
    Rectangle<uint> mainControlArea;
    ScopedPointer<BlendishKnob> blendishMainControl;

    // auxiliary option
    Rectangle<uint> auxOptionArea;
    ScopedPointer<BlendishButtonGroup> blendishAuxOptionButtonGroup;
    ScopedPointer<BlendishCheckBox> blendishAuxOptionCheckBox;
    ScopedPointer<BlendishComboBox> blendishAuxOptionComboBox;
    ScopedPointer<BlendishLabel> blendishAuxOptionLabel;
    const OneKnobAuxiliaryButtonGroupValue* blendishAuxButtonGroupValues;
    const OneKnobAuxiliaryComboBoxValue* blendishAuxComboBoxValues;

    // metering
    BlendishLabel blendishMeterInLabel;
    BlendishLabel blendishMeterOutLabel;
    BlendishMeterLine blendishMeterOutLine;
    BlendishMeterLine blendishMeterInLine;

    Rectangle<uint> getScaledArea(const Rectangle<uint>& area) const
    {
        const double scaleFactor = getScaleFactor();

        if (scaleFactor == 1.0)
            return area;

        Rectangle<uint> copy(area);
        copy.setX(copy.getX() * scaleFactor);
        copy.setY(copy.getY() * scaleFactor);
        copy.growBy(scaleFactor);
        return copy;
    }

    void blendishButtonGroupClicked(BlendishButtonGroup* const buttonGroup, uint button) override
    {
        setParameterValue(buttonGroup->getId(), button);
    }

    void blendishComboBoxIndexChanged(BlendishComboBox* const comboBox, int index) override
    {
        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
            label->setLabel(blendishAuxComboBoxValues[index].description, false);

        setParameterValue(comboBox->getId(), index);
    }

    void buttonClicked(SubWidget* const widget, int) override
    {
        if (blendishAuxOptionCheckBox == widget)
            if (BlendishCheckBox* const checkBox = blendishAuxOptionCheckBox.get())
                setParameterValue(checkBox->getId(), checkBox->isChecked() ? 1.0f : 0.0f);
    }

    void knobDragStarted(SubWidget* const widget) override
    {
        if (blendishMainControl == widget)
            if (BlendishKnob* const knob = blendishMainControl.get())
                editParameter(knob->getId(), true);
    }

    void knobDragFinished(SubWidget* const widget) override
    {
        if (blendishMainControl == widget)
            if (BlendishKnob* const knob = blendishMainControl.get())
                editParameter(knob->getId(), false);
    }

    void knobValueChanged(SubWidget* const widget, const float value) override
    {
        if (blendishMainControl == widget)
            if (BlendishKnob* const knob = blendishMainControl.get())
                setParameterValue(knob->getId(), value);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobUI)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
