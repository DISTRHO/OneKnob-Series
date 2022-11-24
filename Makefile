#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

include dpf/Makefile.base.mk

all: dgl plugins gen

# --------------------------------------------------------------

dgl:
ifeq ($(HAVE_OPENGL),true)
	$(MAKE) -C dpf/dgl opengl
endif

plugins: dgl
	$(MAKE) all -C plugins/AB-InputSelector
	$(MAKE) all -C plugins/AB-OutputSelector
	$(MAKE) all -C plugins/BrickwallLimiter
	$(MAKE) all -C plugins/Compressor
	$(MAKE) all -C plugins/ConvolutionReverb
	$(MAKE) all -C plugins/DevilDistortion
# 	$(MAKE) all -C plugins/Sampler

ifneq ($(CROSS_COMPILING),true)
gen: plugins dpf/utils/lv2_ttl_generator
	@$(CURDIR)/dpf/utils/generate-ttl.sh

dpf/utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen:
endif

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C plugins/AB-InputSelector
	$(MAKE) clean -C plugins/AB-OutputSelector
	$(MAKE) clean -C plugins/BrickwallLimiter
	$(MAKE) clean -C plugins/Compressor
	$(MAKE) clean -C plugins/ConvolutionReverb
	$(MAKE) clean -C plugins/DevilDistortion
# 	$(MAKE) clean -C plugins/Sampler
	rm -rf bin build dpf-widgets/opengl/*.d dpf-widgets/opengl/*.o
	rm -f 3rd-party/FFTConvolver/*.d 3rd-party/FFTConvolver/*.o

# --------------------------------------------------------------

.PHONY: plugins
