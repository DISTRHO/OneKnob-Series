#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = OK-DevilDistortion

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	OneKnobPlugin.cpp

FILES_UI = \
	OneKnobUI.cpp \
	../../dpf-widgets/opengl/Blendish.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

BUILD_CXX_FLAGS += -I../common
BUILD_CXX_FLAGS += -I../../dpf-widgets/opengl

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += jack
TARGETS += ladspa
TARGETS += vst

# ifeq ($(HAVE_OPENGL),true)
# ifeq ($(HAVE_LIBLO),true)
# TARGETS += dssi
# endif
# endif

# ifeq ($(HAVE_OPENGL),true)
# TARGETS += lv2_sep
# else
# TARGETS += lv2_dsp
# endif
TARGETS += lv2

all: $(TARGETS)

# --------------------------------------------------------------
