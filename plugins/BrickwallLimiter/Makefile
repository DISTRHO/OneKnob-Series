#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = OK-BrickwallLimiter

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
LINK_FLAGS      += $(SHARED_MEMORY_LIBS)

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
