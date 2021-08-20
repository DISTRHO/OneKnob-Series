/*
 * DISTRHO OneKnob Series
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cmath>

class ExpSmoother {
public:
    void setSampleRate(float newSampleRate) noexcept
    {
        if (fSampleRate != newSampleRate) {
            fSampleRate = newSampleRate;
            updateCoef();
        }
    }

    void setTimeConstant(float newT60) noexcept
    {
        float newTau = newT60 * (float)(1.0 / 6.91);
        if (fTau != newTau) {
            fTau = newTau;
            updateCoef();
        }
    }

    float getCurrentValue() const noexcept
    {
        return fMem;
    }

    float getTarget() const noexcept
    {
        return fTarget;
    }

    void setTarget(float target) noexcept
    {
        fTarget = target;
    }

    void clear() noexcept
    {
        fMem = 0.0f;
    }

    void clearToTarget() noexcept
    {
        fMem = fTarget;
    }

    float next() noexcept
    {
        float coef = fCoef;
        return (fMem = fMem * coef + fTarget * (1.0f - coef));
    }

private:
    void updateCoef() noexcept
    {
        fCoef = std::exp(-1.0f / (fTau * fSampleRate));
    }

private:
    float fCoef = 0.0f;
    float fTarget = 0.0f;
    float fMem = 0.0f;
    float fTau = 1e-3f;
    float fSampleRate = 44100.0f;
};
