/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2021 Filipe Coelho <falktx@falktx.com>
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

#ifndef DISTRHO_FLOAT_FIFO_HPP_INCLUDED
#define DISTRHO_FLOAT_FIFO_HPP_INCLUDED

#include "DistrhoUtils.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

struct FloatFifo {
   /**
      Pointer to buffer data.
      This can be either stack or heap data, depending on the usecase.
    */
    float* buffer;

   /**
      Current reading position.
      Increments when reading.
    */
    float* readPointer;

   /**
      Current writing position.
      Increments when writing.
    */
    float* writePointer;

   /**
      Amount of float/audio samples in the buffer.
    */
    uint32_t numSamples;

   /**
      Counters used to track available space.
    */
    uint32_t readCounter, writeCounter;
};

// -----------------------------------------------------------------------

/**
   DPF built-in FloatFifo class.
   FloatFifoControl takes one fifo struct to take control over, and operates over it.

   This is meant for single-writer, single-reader type of control.
   Writing and reading is wait and lock-free.

   Typically usage involves:
   ```
   // definition
   HeapFloatFifo myFifo;

   // construction for 512 samples, only needed for heap buffers
   myFifo.alloc(512);

   // writing data
   myFifo.write(0.0f);
   myFifo.write(0.5f);
   myFifo.write(1.0f);

   // reading data
   if (myFifo.readSpace())
   {
      const float value = myFifo.read();
      // do something with "value"
   }
   ```

   @see FloatFifo
 */
class FloatFifoControl
{
public:
    /*
     * Constructor for unitialized float fifo.
     * A call to setFloatFifo is required to tied this control to a float fifo struct;
     *
     */
    FloatFifoControl()
        : fifoPtr(nullptr) {}

    /*
     * Destructor.
     */
    virtual ~FloatFifoControl() {}

    // -------------------------------------------------------------------
    // check operations

    inline uint32_t readSpace()
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifoPtr != nullptr, 0);

        return fifoPtr->numSamples - writeSpace();
    }

    inline uint32_t writeSpace()
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifoPtr != nullptr, 0);

        return fifoPtr->writeCounter - fifoPtr->readCounter;
    }

    // -------------------------------------------------------------------
    // clear/reset operations

    /*
     * Clear the entire float fifo data, marking the fifo as empty.
     * Requires a fifo struct tied to this class.
     */
    void clearData() noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifoPtr != nullptr,);

        fifoPtr->readPointer = fifoPtr->writePointer = fifoPtr->buffer;
        fifoPtr->readCounter = fifoPtr->writeCounter = 0;
        std::memset(fifoPtr->buffer, 0, sizeof(float)*fifoPtr->numSamples);
    }

    // -------------------------------------------------------------------

    /*
     * Tie this float fifo control to a float fifo struct, optionally clearing its data.
     */
    void setFloatFifo(FloatFifo* const floatFifo, const bool clearFifoData) noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifoPtr != floatFifo,);

        fifoPtr = floatFifo;

        if (clearFifoData && floatFifo != nullptr)
            clearData();
    }

    // -------------------------------------------------------------------

    float read()
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifoPtr != nullptr, 0.0f);

        float* tmp = fifoPtr->readPointer;
        const float ret = *tmp;

        if (++tmp == fifoPtr->buffer + fifoPtr->numSamples)
            tmp = fifoPtr->buffer;

        ++fifoPtr->readCounter;
        fifoPtr->readPointer = tmp;
        return ret;
    }

    void write(const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifoPtr != nullptr,);

        float* tmp = fifoPtr->writePointer;

        *tmp = value;

        if (++tmp == fifoPtr->buffer + fifoPtr->numSamples)
            tmp = fifoPtr->buffer;

        ++fifoPtr->writeCounter;
        fifoPtr->writePointer = tmp;
    }

    // -------------------------------------------------------------------

private:
    /** Fifo struct pointer. */
    FloatFifo* fifoPtr;

    DISTRHO_PREVENT_VIRTUAL_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(FloatFifoControl)
};

// -----------------------------------------------------------------------
// FloatFifo using heap space

#ifdef DISTRHO_PROPER_CPP11_SUPPORT
# define FloatFifo_INIT  {nullptr, nullptr, nullptr, 0, 0, 0}
#else
# define FloatFifo_INIT
#endif

/**
   FloatFifoControl with a heap buffer.
   This is a convenience class that provides a method for creating and destroying the heap data.
   Requires the use of alloc(uint32_t) to make the float fifo usable.
*/
class HeapFloatFifo : public FloatFifoControl
{
public:
    /** Constructor. */
    HeapFloatFifo() noexcept
        : fifo(FloatFifo_INIT)
    {
#ifndef DISTRHO_PROPER_CPP11_SUPPORT
        std::memset(&fifo, 0, sizeof(fifo));
#endif
    }

    /** Destructor. */
    ~HeapFloatFifo() noexcept override
    {
        free();
    }

    /** Create a buffer of the specified size. */
    bool alloc(const uint32_t numSamples) noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fifo.buffer == nullptr, false);
        DISTRHO_SAFE_ASSERT_RETURN(numSamples > 0,  false);

        try {
            fifo.buffer = new float[numSamples];
        } DISTRHO_SAFE_EXCEPTION_RETURN("HeapFloatFifo::createBuffer", false);

        fifo.numSamples = numSamples;
        setFloatFifo(&fifo, true);
        return true;
    }

    /** Delete the previously allocated buffer. */
    void free() noexcept
    {
        if (fifo.buffer == nullptr)
            return;

        setFloatFifo(nullptr, false);

        delete[] fifo.buffer;
        fifo.buffer = fifo.readPointer = fifo.writePointer = nullptr;
        fifo.numSamples = fifo.readCounter = fifo.writeCounter = 0;
    }

private:
    /** The heap buffer used for this class. */
    FloatFifo fifo;

    DISTRHO_PREVENT_VIRTUAL_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(HeapFloatFifo)
};

// -----------------------------------------------------------------------
// FloatFifo using stack space

/**
   FloatFifoControl with a stack buffer.
   No setup is necessary, this class is usable as-is.
*/
template<int numSamples>
class StackFloatFifo : public FloatFifoControl
{
public:
    /** Constructor. */
    StackFloatFifo() noexcept
        : fifo(FloatFifo_INIT)
    {
#ifndef DISTRHO_PROPER_CPP11_SUPPORT
        std::memset(&fifo, 0, sizeof(fifo));
#endif
        fifo.buffer = fifoBuffer;
        fifo.numSamples = numSamples;
        setFloatFifo(&fifo, true);
    }

private:
    /** The small stack buffer used for this class. */
    FloatFifo fifo;
    float fifoBuffer[numSamples];

    DISTRHO_PREVENT_VIRTUAL_HEAP_ALLOCATION
    DISTRHO_DECLARE_NON_COPYABLE(StackFloatFifo)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_FLOAT_FIFO_HPP_INCLUDED
