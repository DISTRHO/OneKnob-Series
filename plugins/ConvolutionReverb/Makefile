#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

ifeq ($(MOD_BUILD),true)
NAME = MOD-ConvolutionLoader
else
NAME = OK-ConvolutionReverb
endif

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	OneKnobPlugin.cpp \
	3rd-party.cpp \
	../../3rd-party/FFTConvolver/AudioFFT.cpp \
	../../3rd-party/FFTConvolver/FFTConvolver.cpp \
	../../3rd-party/FFTConvolver/TwoStageFFTConvolver.cpp \
	../../3rd-party/FFTConvolver/Utilities.cpp

FILES_UI = \
	OneKnobUI.cpp \
	../../dpf-widgets/opengl/Blendish.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

BUILD_CXX_FLAGS += -DAUDIOFFT_PFFFT
BUILD_CXX_FLAGS += -DR8B_FASTTIMING
BUILD_CXX_FLAGS += -DR8B_R8B_PFFFT
BUILD_CXX_FLAGS += -I../common
BUILD_CXX_FLAGS += -I../../3rd-party
BUILD_CXX_FLAGS += -I../../3rd-party/FFTConvolver
BUILD_CXX_FLAGS += -I../../3rd-party/r8brain
BUILD_CXX_FLAGS += -I../../dpf-widgets/opengl
BUILD_CXX_FLAGS += -pthread
LINK_FLAGS      += $(SHARED_MEMORY_LIBS)

# BUILD_CXX_FLAGS += -DAUDIOFFT_FFTW3 $(shell $(PKG_CONFIG) --cflags fftw3f)
# LINK_FLAGS      += $(shell $(PKG_CONFIG) --libs fftw3f)

# BUILD_CXX_FLAGS += -DAUDIOFFT_PFFFT -DPFFFT_ENABLE_NEON -Ipffft

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += lv2_sep

ifeq ($(MOD_BUILD),true)
TARGETS += modgui
else
TARGETS += jack
TARGETS += ladspa
TARGETS += vst2
TARGETS += vst3
TARGETS += clap
ifeq ($(HAVE_OPENGL)$(HAVE_LIBLO),truetrue)
TARGETS += dssi
endif
endif

all: $(TARGETS)

# --------------------------------------------------------------
