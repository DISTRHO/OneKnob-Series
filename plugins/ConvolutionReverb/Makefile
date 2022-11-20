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
	dr_wav.cpp \
	FFTConvolver/AudioFFT.cpp \
	FFTConvolver/FFTConvolver.cpp \
	FFTConvolver/TwoStageFFTConvolver.cpp \
	FFTConvolver/Utilities.cpp

# FILES_UI = \
# 	OneKnobUI.cpp \
# 	../../dpf-widgets/opengl/Blendish.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

BUILD_CXX_FLAGS += -I../common
BUILD_CXX_FLAGS += -I../../dpf-widgets/opengl
BUILD_CXX_FLAGS += -IFFTConvolver
LINK_FLAGS      += $(SHARED_MEMORY_LIBS)

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += jack
TARGETS += ladspa
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
