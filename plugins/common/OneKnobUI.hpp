/*
 * DISTRHO OneKnob Compressor
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * For a full copy of the license see the LICENSE file.
 */

#pragma once

#include "DistrhoUI.hpp"

#include "Blendish.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kSidePanelWidth = 24;
static const uint kTopPanelHeight = 32;

// --------------------------------------------------------------------------------------------------------------------

struct OneKnobMainControl {
    uint id;
    const char* label;
    const char* unit;
    float min, max;
};

struct OneKnobAuxiliaryOptionValue {
    int value;
    const char* label;
    const char* description;
};

struct OneKnobAuxiliaryOption {
    uint id;
    const char* title;
    uint count;
    const OneKnobAuxiliaryOptionValue* values;
};

// --------------------------------------------------------------------------------------------------------------------

class OneKnobUI : public UI,
                  public BlendishSubWidget::Callback,
                  public BlendishComboBox::Callback
{
public:
    OneKnobUI(const uint width, const uint height)
        : UI(width, height),
          blendish(this),
          blendishTopPanelMenu(&blendish),
          blendishAuxOptionValues(nullptr)
    {
        blendish.setScaleFactor(getScaleFactor() * 2);
        blendish.setSize(width, height);
        blendishTopPanelMenu.setLabel(DISTRHO_PLUGIN_BRAND " " DISTRHO_PLUGIN_NAME);
    }

    // ----------------------------------------------------------------------------------------------------------------

protected:
    void onDisplay() override
    {
        const GraphicsContext& context(getGraphicsContext());
        const uint width  = getWidth();
        const uint height = getHeight();

        Color(0.4f, 0.3f, 0.2f).setFor(context);
        Rectangle<uint>(0, 0, kSidePanelWidth, height).draw(context);
        Rectangle<uint>(width-kSidePanelWidth, 0, kSidePanelWidth, height).draw(context);

        Color(0.2f, 0.3f, 0.4f).setFor(context);
        Rectangle<uint>(kSidePanelWidth, 0, width - kSidePanelWidth * 2, height).draw(context);
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

    void createMainControl(const Rectangle<uint>& area, const OneKnobMainControl& control)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishMainControl == nullptr,);

        BlendishNumberField* const knob = new BlendishNumberField(&blendish);
        knob->setId(control.id);
        knob->setLabel(control.label);

        mainControlArea = area;
        blendishMainControl = knob;
    }

    void setMainControlValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishMainControl != nullptr,);

        blendishMainControl->setValue(value);
    }

    // ----------------------------------------------------------------------------------------------------------------

    void createAuxiliaryOption(const Rectangle<uint>& area, const OneKnobAuxiliaryOption& option)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionComboBox == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionValues == nullptr,);
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

        label->setLabel("Loading...");
        label->setId(option.id);

        auxOptionArea = area;
        blendishAuxOptionValues = option.values;
        blendishAuxOptionComboBox = comboBox;
        blendishAuxOptionLabel = label;
    }

    void setAuxiliaryOptionValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionComboBox != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionValues != nullptr,);

        const int index = static_cast<int>(value + 0.5f);
        DISTRHO_SAFE_ASSERT_INT_RETURN(index >= 0 && index <= 4, index,); // FIXME max range

        blendishAuxOptionComboBox->setCurrentIndex(index, false);
        blendishAuxOptionLabel->setLabel(blendishAuxOptionValues[index].description);
    }

    // ----------------------------------------------------------------------------------------------------------------

    void repositionWidgets()
    {
        const uint width = getWidth();

        // top panel
        topPanelArea.setX(kSidePanelWidth * 2);
        topPanelArea.setSize(width - kSidePanelWidth * 4, kTopPanelHeight);
        blendishTopPanelMenu.setAbsoluteX(topPanelArea.getX() / 2 - 4);
        blendishTopPanelMenu.setAbsoluteY(-3);
        blendishTopPanelMenu.setWidth(topPanelArea.getWidth() / 2);

        // main control
        if (BlendishNumberField* const knob = blendishMainControl.get())
        {
            knob->setAbsoluteX(mainControlArea.getX());
            knob->setAbsoluteY(mainControlArea.getY());
        }

        // auxiliary options
        uint comboBoxHeight = 0;

        if (BlendishComboBox* const comboBox = blendishAuxOptionComboBox.get())
        {
            comboBoxHeight = comboBox->getHeight();
            comboBox->setAbsoluteX(auxOptionArea.getX());
            comboBox->setAbsoluteY(auxOptionArea.getY());
        }

        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
        {
            label->setAbsoluteX(auxOptionArea.getX());
            label->setAbsoluteY(auxOptionArea.getY() + comboBoxHeight + 2);
        }
    }

private:
    BlendishSubWidgetSharedContext blendish;

    // top panel
    Rectangle<uint> topPanelArea;
    BlendishMenu blendishTopPanelMenu;

    // main knob
    Rectangle<uint> mainControlArea;
    ScopedPointer<BlendishNumberField> blendishMainControl;

    // auxiliary option
    Rectangle<uint> auxOptionArea;
    ScopedPointer<BlendishComboBox> blendishAuxOptionComboBox;
    ScopedPointer<BlendishLabel> blendishAuxOptionLabel;
    const OneKnobAuxiliaryOptionValue* blendishAuxOptionValues;

    void blendishWidgetClicked(BlendishSubWidget* const widget, const int button) override
    {
        d_stdout("blendishWidgetClicked %p %i %i", widget, widget->getId(), button);
    }

    void blendishComboBoxIndexChanged(BlendishComboBox* const comboBox, int index) override
    {
        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
            label->setLabel(blendishAuxOptionValues[index].description);

        setParameterValue(comboBox->getId(), index);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobUI)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
