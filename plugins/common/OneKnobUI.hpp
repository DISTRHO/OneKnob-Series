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

// IDE helper (not needed for building)
#include "OpenGL.hpp"

#include "DistrhoUI.hpp"

#include "Blendish.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

static const uint kSidePanelWidth = 12;
static const uint kTopPanelHeight = 32;

// --------------------------------------------------------------------------------------------------------------------

struct OneKnobMainControl {
    uint id;
    const char* label;
    const char* unit;
    float min, max;
};

struct OneKnobAuxiliaryCheckBox {
    uint id;
    const char* title;
    const char* description;
};

struct OneKnobAuxiliaryComboBoxValue {
    int value;
    const char* label;
    const char* description;
};

struct OneKnobAuxiliaryComboBox {
    uint id;
    const char* title;
    uint count;
    const OneKnobAuxiliaryComboBoxValue* values;
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
          blendishTopLabel(&blendish),
          /*
          blendishTopPanelMenu(&blendish),
          menuAnimatingOpen(-1),
          menuAnimatingClose(-1),
          */
          blendishAuxComboBoxValues(nullptr)
    {
        const double scaleFactor = getScaleFactor();

        if (scaleFactor != 1.0)
            setSize(width * scaleFactor, height * scaleFactor);

        blendish.setScaleFactor(scaleFactor * 2);
        blendish.setSize(width * scaleFactor, height * scaleFactor);
        blendishTopLabel.setLabel(DISTRHO_PLUGIN_BRAND " " DISTRHO_PLUGIN_NAME);
        /*
        blendishTopPanelMenu.setLabel(DISTRHO_PLUGIN_BRAND " " DISTRHO_PLUGIN_NAME);
        */

        // TESTING
        lineWriteIndex = 0;
        std::memset(lines, 0, sizeof(lines));
    }

    // ----------------------------------------------------------------------------------------------------------------

protected:
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
        Rectangle<uint>(kSidePanelWidth,
                        kSidePanelWidth,
                        width - kSidePanelWidth * 2,
                        height - kSidePanelWidth * 2).drawOutline(context);

        // frame outer
        Color::fromHTML("#000000").setFor(context);
        Rectangle<uint>(kSidePanelWidth - 1,
                        kSidePanelWidth - 1,
                        width - kSidePanelWidth * 2 + 2,
                        height - kSidePanelWidth * 2 + 2).drawOutline(context);

        // brand line
        {
            const uint y = kSidePanelWidth * scaleFactor + blendishTopLabel.getHeight() * 2 + 2 * scaleFactor;
            // for text use #cacacb
            Color::fromHTML("#3b393d").setFor(context);
            Line<uint>(kSidePanelWidth + 4, y, width - kSidePanelWidth - 4, y).draw(context);
        }

        // flow line
        Color::fromHTML("#665d98").setFor(context);
        glLineWidth(1.0f);

        // TESTING
        const int size = sizeof(lines)/sizeof(lines[0]);
        const int startX = kSidePanelWidth * 2 * scaleFactor;
        const int startY = height - (kSidePanelWidth * 2 * scaleFactor);
        int k = lineWriteIndex;

        glLineWidth(scaleFactor);
        glScissor(startX, kSidePanelWidth * 2 * scaleFactor, size * scaleFactor, 80 * scaleFactor);
        glEnable(GL_SCISSOR_TEST);
        glBegin(GL_LINE_LOOP);

        glVertex2d(kSidePanelWidth, height);
        glVertex2d(kSidePanelWidth, startY);
        glVertex2d(kSidePanelWidth, startY - lines[k] * 80 * scaleFactor);

        for (int i=0; i<size; ++i, ++k)
        {
            if (k == size)
                k = 0;
            glVertex2d(startX + i * scaleFactor, startY - lines[k] * 80 * scaleFactor);
        }

        glVertex2d(startX + size * scaleFactor, startY - lines[k-1] * 80 * scaleFactor);
        glVertex2d(width - kSidePanelWidth, startY - lines[k-1] * 80 * scaleFactor);
        glVertex2d(width - kSidePanelWidth, height);

        glEnd();
        glDisable(GL_SCISSOR_TEST);
    }

    bool onMouse(const MouseEvent& ev) override
    {
        // Point<double> pos2(ev.pos.getX() / 2, ev.pos.getY() / 2);

        // FIXME wrong relative pos, should check click via widget instead
        /*
        if (ev.press && blendishTopPanelMenu.contains(ev.pos) && menuAnimatingOpen == -1 && menuAnimatingClose == -1)
        {
            const double scaleFactor = getScaleFactor();

            blendishTopPanelMenu.toFront();

            if (blendishTopPanelMenu.getHeight() > getHeight()/4)
                menuAnimatingClose = (21 * scaleFactor);
            else
                menuAnimatingOpen = getHeight() - (kSidePanelWidth * 7 * scaleFactor);

            d_stdout("animating");
            repaint();
            return true;
        }
        */

        return UI::onMouse(ev);
    }

    // ----------------------------------------------------------------------------------------------------------------

    void uiIdle() override
    {
        /*
        const double scaleFactor = getScaleFactor();

        if (menuAnimatingOpen != -1)
        {
            const int curHeight = blendishTopPanelMenu.getHeight();

            if (curHeight >= menuAnimatingOpen)
            {
                menuAnimatingOpen = -1;
                return;
            }

            blendishTopPanelMenu.setHeight(std::min((double)menuAnimatingOpen, curHeight + (scaleFactor * 25)));
            repaint();
        }
        else if (menuAnimatingClose != -1)
        {
            const int curHeight = blendishTopPanelMenu.getHeight();

            if (curHeight <= menuAnimatingClose)
            {
                menuAnimatingClose = -1;
                return;
            }

            blendishTopPanelMenu.setHeight(std::max((double)menuAnimatingClose, curHeight - (scaleFactor * 25)));
            repaint();
        }
        */
    }

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
        knob->setId(control.id);
        knob->setLabel(control.label);

        mainControlArea = getScaledArea(area);
        blendishMainControl = knob;
    }

    void setMainControlValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishMainControl != nullptr,);

        blendishMainControl->setCurrentValue(value);
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

        auxOptionArea = getScaledArea(area);
        blendishAuxOptionCheckBox = checkBox;
        blendishAuxOptionLabel = label;
    }

    void setAuxiliaryCheckBoxValue(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionCheckBox != nullptr,);

        blendishAuxOptionCheckBox->setChecked(value > 0.5f /*, false */);
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
        blendishAuxOptionLabel->setLabel(blendishAuxComboBoxValues[index].description);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // aux text

    void createAuxiliaryText(const Rectangle<uint>& area, const char* const text)
    {
        DISTRHO_SAFE_ASSERT_RETURN(blendishAuxOptionLabel == nullptr,);

        BlendishLabel* const label = new BlendishLabel(&blendish);

        label->setLabel(text);

        auxOptionArea = getScaledArea(area);
        blendishAuxOptionLabel = label;
    }

    // ----------------------------------------------------------------------------------------------------------------

    void repositionWidgets()
    {
        const double scaleFactor = getScaleFactor();
        const uint width = getWidth();

        // top panel
        blendishTopLabel.setAbsoluteX(kSidePanelWidth / 2 - 2);
        blendishTopLabel.setAbsoluteY(kSidePanelWidth / 2);
        blendishTopLabel.setWidth(width - kSidePanelWidth * 2);
        /*
        blendishTopPanelMenu.setAbsoluteX((kSidePanelWidth + 4) * scaleFactor);
        blendishTopPanelMenu.setAbsoluteY(-3 * scaleFactor);
        blendishTopPanelMenu.setWidth(width / (2 / scaleFactor) - (kSidePanelWidth + 4) * 2 * scaleFactor); // FIXME
        blendishTopPanelMenu.setHeight(21 * scaleFactor);
        */

        // main control
        if (BlendishKnob* const knob = blendishMainControl.get())
        {
            knob->setAbsoluteX(mainControlArea.getX());
            knob->setAbsoluteY(mainControlArea.getY());
        }

        // auxiliary options
        uint auxWidgetHeight = 0;

        if (BlendishCheckBox* const checkBox = blendishAuxOptionCheckBox.get())
        {
            auxWidgetHeight = checkBox->getHeight();
            checkBox->setAbsoluteX(auxOptionArea.getX());
            checkBox->setAbsoluteY(auxOptionArea.getY());
        }

        if (BlendishComboBox* const comboBox = blendishAuxOptionComboBox.get())
        {
            auxWidgetHeight = comboBox->getHeight();
            comboBox->setAbsoluteX(auxOptionArea.getX());
            comboBox->setAbsoluteY(auxOptionArea.getY());
        }

        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
        {
            label->setAbsoluteX(auxOptionArea.getX());
            label->setAbsoluteY(auxOptionArea.getY() + auxWidgetHeight + 2);
        }
    }

    // shared, TESTING
    float lines[512];
    int lineWriteIndex;

private:
    BlendishSubWidgetSharedContext blendish;

    // top panel
    /*
    BlendishMenu blendishTopPanelMenu;
    int menuAnimatingOpen;
    int menuAnimatingClose;
    */
    BlendishLabel blendishTopLabel;

    // main knob
    Rectangle<uint> mainControlArea;
    ScopedPointer<BlendishKnob> blendishMainControl;

    // auxiliary option
    Rectangle<uint> auxOptionArea;
    ScopedPointer<BlendishCheckBox> blendishAuxOptionCheckBox;
    ScopedPointer<BlendishComboBox> blendishAuxOptionComboBox;
    ScopedPointer<BlendishLabel> blendishAuxOptionLabel;
    const OneKnobAuxiliaryComboBoxValue* blendishAuxComboBoxValues;

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

    void blendishWidgetClicked(BlendishSubWidget* const widget, int) override
    {
        if (blendishAuxOptionCheckBox == widget)
            if (BlendishCheckBox* const checkBox = blendishAuxOptionCheckBox.get())
                setParameterValue(checkBox->getId(), checkBox->isChecked() ? 1.0f : 0.0f);
    }

    void blendishComboBoxIndexChanged(BlendishComboBox* const comboBox, int index) override
    {
        if (BlendishLabel* const label = blendishAuxOptionLabel.get())
            label->setLabel(blendishAuxComboBoxValues[index].description);

        setParameterValue(comboBox->getId(), index);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneKnobUI)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
