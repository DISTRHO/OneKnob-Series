#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = OK-ConvolutionReverb

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	OneKnobPlugin.cpp \
	3rd-party.cpp \
	FFTConvolver/AudioFFT.cpp \
	FFTConvolver/FFTConvolver.cpp \
	FFTConvolver/TwoStageFFTConvolver.cpp \
	FFTConvolver/Utilities.cpp

FILES_UI = \
	OneKnobUI.cpp \
	../../dpf-widgets/opengl/Blendish.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

BUILD_CXX_FLAGS += -I../common
BUILD_CXX_FLAGS += -I../../dpf-widgets/opengl
BUILD_CXX_FLAGS += -IFFTConvolver
BUILD_CXX_FLAGS += -pthread
LINK_FLAGS      += $(SHARED_MEMORY_LIBS)

# BUILD_CXX_FLAGS += -DAUDIOFFT_FFTW3 $(shell $(PKG_CONFIG) --cflags fftw3f)
# LINK_FLAGS      += $(shell $(PKG_CONFIG) --libs fftw3f)

BUILD_CXX_FLAGS += -DAUDIOFFT_PFFFT -DPFFFT_ENABLE_NEON -Ipffft

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += jack
TARGETS += lv2_sep
TARGETS += vst2
TARGETS += vst3
TARGETS += clap

ifeq ($(HAVE_OPENGL),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif
endif

all: $(TARGETS)

# --------------------------------------------------------------
