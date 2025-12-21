#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = OK-Drummer

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	OneKnobPlugin.cpp \
	../../3rd-party/batteur/BeatDescription.cpp \
	../../3rd-party/batteur/FileReadingHelpers.cpp \
	../../3rd-party/batteur/Player.cpp \
	../../3rd-party/batteur/fmidi/fmidi_mini.cpp \
	../../3rd-party/fluidsynth/src/fluid_adsr_env.c \
	../../3rd-party/fluidsynth/src/fluid_chan.c \
	../../3rd-party/fluidsynth/src/fluid_chorus.c \
	../../3rd-party/fluidsynth/src/fluid_conv.c \
	../../3rd-party/fluidsynth/src/fluid_defsfont.c \
	../../3rd-party/fluidsynth/src/fluid_event.c \
	../../3rd-party/fluidsynth/src/fluid_gen.c \
	../../3rd-party/fluidsynth/src/fluid_hash.c \
	../../3rd-party/fluidsynth/src/fluid_iir_filter.c \
	../../3rd-party/fluidsynth/src/fluid_lfo.c \
	../../3rd-party/fluidsynth/src/fluid_list.c \
	../../3rd-party/fluidsynth/src/fluid_midi.c \
	../../3rd-party/fluidsynth/src/fluid_mod.c \
	../../3rd-party/fluidsynth/src/fluid_rev.c \
	../../3rd-party/fluidsynth/src/fluid_ringbuffer.c \
	../../3rd-party/fluidsynth/src/fluid_rvoice.c \
	../../3rd-party/fluidsynth/src/fluid_rvoice_dsp.c \
	../../3rd-party/fluidsynth/src/fluid_rvoice_event.c \
	../../3rd-party/fluidsynth/src/fluid_rvoice_mixer.c \
	../../3rd-party/fluidsynth/src/fluid_samplecache.c \
	../../3rd-party/fluidsynth/src/fluid_settings.c \
	../../3rd-party/fluidsynth/src/fluid_sffile.c \
	../../3rd-party/fluidsynth/src/fluid_sfont.c \
	../../3rd-party/fluidsynth/src/fluid_synth.c \
	../../3rd-party/fluidsynth/src/fluid_synth_monopoly.c \
	../../3rd-party/fluidsynth/src/fluid_sys.c \
	../../3rd-party/fluidsynth/src/fluid_tuning.c \
	../../3rd-party/fluidsynth/src/fluid_voice.c

# --------------------------------------------------------------
# Extra params

FLUIDSYNTH_BUILD_FLAGS  = -DHAVE_CONFIG_H -DDEFAULT_SOUNDFONT=\"\"
FLUIDSYNTH_BUILD_FLAGS += -DHAVE_COSF=1 -DHAVE_FABSF=1 -DHAVE_LOGF=1 -DHAVE_POWF=1 -DHAVE_SINF=1 -DHAVE_SQRTF=1
FLUIDSYNTH_BUILD_FLAGS += -I../../3rd-party/fluidsynth
FLUIDSYNTH_BUILD_FLAGS += -I../../3rd-party/fluidsynth/fluidsynth
FLUIDSYNTH_BUILD_FLAGS += -I../../3rd-party/fluidsynth/src
FLUIDSYNTH_BUILD_FLAGS += $(shell pkg-config --cflags gthread-2.0 glib-2.0 sndfile)
FLUIDSYNTH_BUILD_FLAGS += -pthread

ifeq ($(WINDOWS),true)
FLUIDSYNTH_BUILD_FLAGS += -DFLUIDSYNTH_NOT_A_DLL -DWIN32
endif

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

BUILD_C_FLAGS   += $(FLUIDSYNTH_BUILD_FLAGS)
BUILD_C_FLAGS   += -Wno-unknown-pragmas
BUILD_C_FLAGS   += -Wno-unused-function
BUILD_C_FLAGS   += -Wno-unused-parameter

BUILD_CXX_FLAGS += $(FLUIDSYNTH_BUILD_FLAGS)
BUILD_CXX_FLAGS += -DFMIDI_DISABLE_DESCRIBE_API=1
BUILD_CXX_FLAGS += -DFMIDI_STATIC=1
BUILD_CXX_FLAGS += -std=gnu++17
BUILD_CXX_FLAGS += -I../common
BUILD_CXX_FLAGS += -I../../3rd-party/batteur
BUILD_CXX_FLAGS += -fno-finite-math-only

LINK_FLAGS      += $(shell pkg-config --libs gthread-2.0 glib-2.0 sndfile)
LINK_FLAGS      += -pthread
LINK_FLAGS      += $(SHARED_MEMORY_LIBS)

ifeq ($(WINDOWS),true)
LINK_FLAGS      += -lole32 -lws2_32
endif

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += lv2_sep

ifneq ($(MOD_BUILD),true)
TARGETS += clap
TARGETS += vst2
TARGETS += vst3
endif

all: $(TARGETS)

# --------------------------------------------------------------
