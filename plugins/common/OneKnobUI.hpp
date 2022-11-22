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
#include "SharedMemory.hpp"
#include "Blendish.hpp"

START_NAMESPACE_DISTRHO

// line below fixes windows build
using DGL_NAMESPACE::Rectangle;

// --------------------------------------------------------------------------------------------------------------------

static inline MATH_CONSTEXPR int lin2dbint(const float value)
{
    return static_cast<int>(20.0f * std::log10(value) - 0.5f);
}

// --------------------------------------------------------------------------------------------------------------------

static const uint kSidePanelWidth = 12;
static const char* kDefaultLineMeterNames[2] = { "In:", "Out:" };

// --------------------------------------------------------------------------------------------------------------------

struct ThemeInitializer {
    ThemeInitializer()
    {
        const Color bg(Color::fromHTML("#222126"));
        const Color fg(0xf0, 0xf0, 0xf0);

        const BlendishWidgetTheme w = {
            bg.plus(0x50), // outlineColor: color of widget box outline
            fg,            // itemColor: color of widget item (meaning changes depending on class)
            bg.plus(0x10), // innerColor: fill color of widget box
            bg.plus(0x30), // innerSelectedColor: fill color of widget box when active
            fg,            // textColor: color of text label
            fg,            // textSelectedColor: color of text label when active
            0,             // shadeTop: delta modifier for upper part of gradient (-100 to 100)
            0,             // shadeDown: delta modifier for lower part of gradient (-100 to 100)
        };

        BlendishTheme theme = {
            Color(0,0,0,0.f), // backgroundColor: the background color of panels and windows
            w,             // labelTheme: theme for labels
            w,             // toolButtonTheme: theme for tool buttons
           #if 0
            w,             // radioButtonTheme: theme for radio buttons
            w,             // textFieldTheme: theme for text fields
            /*
            = sliderTheme;
            textColor = textSelectedColor = bg.minus(0x20);
            itemColor = bg.plus(0x30);
            */
           #endif
            w,             // checkBoxTheme: theme for checkboxes (option buttons)
            w,             // comboBoxTheme: theme for comboboxes (choice buttons), Blender calls them "menu buttons"
            w,             // numberFieldTheme: theme for number fields
           #if 0
            w,             // sliderTheme: theme for slider controls
            /*
            itemColor = bg;
            innerColor = bg.plus(0x50);
            innerSelectedColor = bg.plus(0x60);
            */
            w,             // scrollBarTheme: theme for scrollbars
            /*
            itemColor = bg.plus(0x50);
            innerColor = bg;
            */
            w,             // tooltipTheme: theme for tooltips
           #endif
            w,             // menuTheme: theme for menu backgrounds
            w,             // menuItemTheme: theme for menu items
           #if 0
            w,             // nodeTheme: theme for nodes
           #endif
        };

        theme.menuTheme.innerColor = bg; // .minus(0x10);
        theme.menuTheme.textColor = theme.menuTheme.textSelectedColor = fg.minus(0x50);

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
        lines[writeIndex] = value;
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
                  public BlendishToolButton::Callback,
                  public ButtonEventHandler::Callback,
                  public KnobEventHandler::Callback,
                  public IdleCallback
{
public:
    OneKnobUI(const uint width, const uint height, const char* lineMeterNames[2] = kDefaultLineMeterNames)
        : UI(width, height),
          tisi(),
          blendish(this),
          blendishTopLabel(&blendish),
          blendishAuxButtonGroupValues(nullptr),
          blendishAuxComboBoxValues(nullptr),
          blendishAuxFileButtonKey(nullptr),
          blendishMeter1Label(&blendish),
          blendishMeter1LabelValue(&blendish),
          blendishMeter2Label(&blendish),
          blendishMeter2LabelValue(&blendish),
          blendishMeter2Line(&blendish, Color::fromHTML("#c90054")),
          blendishMeter1Line(&blendish, Color(0x3E, 0xB8, 0xBE, 0.75f)),
          firstIdle(true)
    {
        const double scaleFactor = getScaleFactor();

        if (scaleFactor != 1.0)
            setSize(width * scaleFactor, height * scaleFactor);

        blendish.setScaleFactor(scaleFactor * 2);
        blendish.setSize(width * scaleFactor, height * scaleFactor);
        blendishTopLabel.setColor(Color(1.0, 1.0f, 1.0f));
        blendishTopLabel.setLabel(DISTRHO_PLUGIN_BRAND " " DISTRHO_PLUGIN_NAME);

        blendishMeter1Label.setColor(Color(0x3E, 0xB8, 0xBE, 0.75f));
        blendishMeter1Label.setLabel(lineMeterNames[0]);
        blendishMeter1Label.setFontSize(8);

        blendishMeter1LabelValue.setAlignment(BlendishLabel::kAlignmentRight);
        blendishMeter1LabelValue.setColor(Color(0x3E, 0xB8, 0xBE, 0.75f));
        blendishMeter1LabelValue.setLabel("-inf dB");
        blendishMeter1LabelValue.setFontSize(8);

        blendishMeter2Label.setColor(Color::fromHTML("#c90054"));
        blendishMeter2Label.setLabel(lineMeterNames[1]);
        blendishMeter2Label.setFontSize(8);

        blendishMeter2LabelValue.setAlignment(BlendishLabel::kAlignmentRight);
        blendishMeter2LabelValue.setColor(Color::fromHTML("#c90054"));
        blendishMeter2LabelValue.setLabel("-inf dB");
        blendishMeter2LabelValue.setFontSize(8);
    }

    ~OneKnobUI() override
    {
        if (lineGraphsData.isCreatedOrConnected())
        {
            removeIdleCallback(this);
            lineGraphsData.getDataPointer()->closed = true;
            lineGraphsData.close();
        }
    }

protected:
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
        Rectangle<int>(blendishMeter1Line.getAbsoluteX() * 2 * scaleFactor,
                       blendishMeter1Line.getAbsoluteY() * 2 * scaleFactor,
                       blendishMeter1Line.getWidth() * 2 * scaleFactor,
                       blendishMeter1Line.getHeight() * 2 * scaleFactor).drawOutline(context);
    }

    // ----------------------------------------------------------------------------------------------------------------

    // TESTING
    bool onKeyboard(const KeyboardEvent&) override
    {
        getWindow().renderToPicture("testing.ppm");
        return true;
    }

    void uiIdle() override
    {
        if (firstIdle)
        {
            firstIdle = false;

            if (! lineGraphsData.create())
                return;

            OneKnobLineGraphFifos* const fifos = lineGraphsData.getDataPointer();
            lineGraph1.setFloatFifo(&fifos->v1, true);
            lineGraph2.setFloatFifo(&fifos->v2, true);

            setState("filemapping", lineGraphsData.getDataFilename());
            addIdleCallback(this, 1000 / 60); // 60fps
        }
    }

    void idleCallback() override
    {
        bool shouldRepaint = false;

        if (lineGraph1.canRead())
        {
            pushInputMeter(lineGraph1.read());
            shouldRepaint = true;
        }

        if (lineGraph2.canRead())
        {
            pushOutputMeter(lineGraph2.read());
            shouldRepaint = true;
        }

        if (shouldRepaint)
            repaint();
    }

    void uiFocus(const bool focus, CrossingMode) override
    {
        if (focus)
            return;

        // fake event at (0,0) to close any open menus
        {
            MouseEvent ev;
            ev.button = kMouseButtonLeft;
            onMouse(ev);
        }

        // fake event at (0,0) to remove any hover state
        {
            MotionEvent ev;
            onMotion(ev);
        }
    }

    void stateChanged(const char* const key, const char* const value) override
    {
        if (blendishAuxFileButtonKey != nullptr && value != nullptr)
            if (std::strcmp(key, blendishAuxFileButtonKey) == 0)
                if (const char* const rvalue = std::strrchr(value, DISTRHO_OS_SEP))
                    if (BlendishLabel* const label = blendishAuxOptionLabel.get())
                        label->setLabel(rvalue + 1);
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
        knob->setUnitLabel(control.unit);
        knob->setUnitColor(Color::fromHTML("#cacacb")); // FIXME label color autmatically

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

    void createAuxiliaryFileButton(const Rectangle<uint>& area, const OneKnobAuxiliaryFileButton& option, const OneKnobAuxiliarySlider& numFieldOpts)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionComboBox == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxFileButtonKey == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(option.key != nullptr,);

        BlendishToolButton* const fileButton = new BlendishToolButton(&blendish);
        BlendishNumberField* const numField = new BlendishNumberField(&blendish);
        BlendishLabel* const label = new BlendishLabel(&blendish);

        numField->setCallback(this);
        numField->setId(numFieldOpts.id);
        numField->setLabel(numFieldOpts.label);
        numField->setRange(numFieldOpts.min, numFieldOpts.max);
        numField->setValue(numFieldOpts.def);

        fileButton->setCallback(this);
        fileButton->setLabel(option.label);

        label->setLabel("(No file loaded yet)");
        label->setFontSize(9);

        auxOptionArea = getScaledArea(area);
        blendishAuxFileButtonKey = option.key;
        blendishAuxOptionFileButton = fileButton;
        blendishAuxOptionNumberField = numField;
        blendishAuxOptionLabel = label;
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
        blendishMeter1Line.push(std::min(1.1f, value));

        if (value < 0.0001f)
            return blendishMeter1LabelValue.setLabel("-inf dB", false);

        char strBuf[0xff];
        snprintf(strBuf, sizeof(strBuf), "%d dB", lin2dbint(value));
        blendishMeter1LabelValue.setLabel(strBuf, false);
    }

    void pushOutputMeter(const float value)
    {
        blendishMeter2Line.push(std::min(1.1f, value));

        if (value < 0.0001f)
            return blendishMeter2LabelValue.setLabel("-inf dB", false);

        char strBuf[0xff];
        snprintf(strBuf, sizeof(strBuf), "%d dB", lin2dbint(value));
        blendishMeter2LabelValue.setLabel(strBuf, false);
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
            buttonGroup->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight);
            auxWidgetHeight += buttonGroup->getHeight() + 2;
            auxWidgetPosX = buttonGroup->getAbsoluteX();
        }

        if (BlendishCheckBox* const checkBox = blendishAuxOptionCheckBox.get())
        {
            checkBox->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - checkBox->getWidth()/2);
            checkBox->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight);
            auxWidgetHeight = checkBox->getHeight();
            auxWidgetPosX = checkBox->getAbsoluteX();
        }

        if (BlendishComboBox* const comboBox = blendishAuxOptionComboBox.get())
        {
            comboBox->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - comboBox->getWidth()/2);
            comboBox->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight);
            auxWidgetHeight = comboBox->getHeight();
            auxWidgetPosX = comboBox->getAbsoluteX();
        }

        if (BlendishNumberField* const numField = blendishAuxOptionNumberField.get())
        {
            numField->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - numField->getWidth()/2);
            numField->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight);
            auxWidgetHeight += numField->getHeight() + 2;
            auxWidgetPosX = auxOptionArea.getX();
        }

        if (BlendishToolButton* const fileButton = blendishAuxOptionFileButton.get())
        {
            fileButton->setAbsoluteX(auxOptionArea.getX() + auxOptionArea.getWidth()/2 - fileButton->getWidth()/2);
            fileButton->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight);
            auxWidgetHeight += fileButton->getHeight();
            auxWidgetPosX = auxOptionArea.getX();
        }

        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
        {
            label->setAbsoluteX(auxWidgetPosX);
            label->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight + 2);
            label->setWidth(auxOptionArea.getWidth() - (auxWidgetPosX - auxOptionArea.getX()));
            label->setHeight(auxOptionArea.getHeight() - auxWidgetHeight - 2);
        }

        // metering
        blendishMeter1Line.setAbsolutePos(kSidePanelWidth,
                                           height / 2 - blendishMeter1Line.getHeight() - kSidePanelWidth);
        blendishMeter2Line.setAbsolutePos(kSidePanelWidth,
                                            height / 2 - blendishMeter2Line.getHeight() - kSidePanelWidth);

        blendishMeter1Label.setAbsolutePos(width * scaleFactor / 2 - 58 * scaleFactor, (height - 240) * scaleFactor);
        blendishMeter2Label.setAbsolutePos(width * scaleFactor / 2 - 58 * scaleFactor, (height - 230) * scaleFactor);

        blendishMeter1LabelValue.setAbsolutePos(blendishMeter1Label.getAbsoluteX() - 14 * scaleFactor,
                                                 blendishMeter1Label.getAbsoluteY());
        blendishMeter2LabelValue.setAbsolutePos(blendishMeter2Label.getAbsoluteX() - 14 * scaleFactor,
                                                  blendishMeter2Label.getAbsoluteY());
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
    ScopedPointer<BlendishNumberField> blendishAuxOptionNumberField;
    ScopedPointer<BlendishToolButton> blendishAuxOptionFileButton;
    ScopedPointer<BlendishLabel> blendishAuxOptionLabel;
    const OneKnobAuxiliaryButtonGroupValue* blendishAuxButtonGroupValues;
    const OneKnobAuxiliaryComboBoxValue* blendishAuxComboBoxValues;
    const char* blendishAuxFileButtonKey;

    // metering
    BlendishLabel blendishMeter1Label;
    BlendishLabel blendishMeter1LabelValue;
    BlendishLabel blendishMeter2Label;
    BlendishLabel blendishMeter2LabelValue;
    BlendishMeterLine blendishMeter2Line;
    BlendishMeterLine blendishMeter1Line;

    // wait until first idle to setup fifo, in case UI is created as test
    bool firstIdle;

    // metering fifo
    OneKnobFloatFifoControl lineGraph1;
    OneKnobFloatFifoControl lineGraph2;
    SharedMemory<OneKnobLineGraphFifos> lineGraphsData;

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

    void blendishToolButtonClicked(BlendishToolButton*, int) override
    {
        if (blendishAuxFileButtonKey != nullptr)
            requestStateFile(blendishAuxFileButtonKey);
    }

    void buttonClicked(SubWidget* const widget, int) override
    {
        if (blendishAuxOptionCheckBox == widget)
            if (BlendishCheckBox* const checkBox = blendishAuxOptionCheckBox.get())
                setParameterValue(checkBox->getId(), checkBox->isChecked() ? 1.0f : 0.0f);
    }

    void knobDragStarted(SubWidget* const widget) override
    {
        editParameter(widget->getId(), true);
    }

    void knobDragFinished(SubWidget* const widget) override
    {
        editParameter(widget->getId(), false);
    }

    void knobValueChanged(SubWidget* const widget, const float value) override
    {
        setParameterValue(widget->getId(), value);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobUI)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
