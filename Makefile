#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

include dpf/Makefile.base.mk

all: dgl plugins gen

# --------------------------------------------------------------

ifeq ($(MOD_BUILD),true)
MAKE_TARGET = lv2_sep
else
MAKE_TARGET = all
endif

dgl:
ifeq ($(HAVE_OPENGL),true)
	$(MAKE) -C dpf/dgl opengl
endif

plugins: dgl
# 	$(MAKE) $(MAKE_TARGET) -C plugins/AB-InputSelector
# 	$(MAKE) $(MAKE_TARGET) -C plugins/AB-OutputSelector
	$(MAKE) $(MAKE_TARGET) -C plugins/BrickwallLimiter
# 	$(MAKE) $(MAKE_TARGET) -C plugins/Compressor
	$(MAKE) $(MAKE_TARGET) -C plugins/ConvolutionReverb
	$(MAKE) $(MAKE_TARGET) -C plugins/DevilDistortion
	$(MAKE) $(MAKE_TARGET) -C plugins/Drummer
	$(MAKE) $(MAKE_TARGET) -C plugins/Filter
# 	$(MAKE) $(MAKE_TARGET) -C plugins/Sampler

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
# 	$(MAKE) clean -C plugins/AB-InputSelector
# 	$(MAKE) clean -C plugins/AB-OutputSelector
	$(MAKE) clean -C plugins/BrickwallLimiter
# 	$(MAKE) clean -C plugins/Compressor
	$(MAKE) clean -C plugins/ConvolutionReverb
	$(MAKE) clean -C plugins/DevilDistortion
	$(MAKE) clean -C plugins/Drummer
	$(MAKE) clean -C plugins/Filter
# 	$(MAKE) clean -C plugins/Sampler
	rm -rf bin build dpf-widgets/opengl/*.d dpf-widgets/opengl/*.o
	rm -f 3rd-party/FFTConvolver/*.d 3rd-party/FFTConvolver/*.o

# --------------------------------------------------------------

.PHONY: plugins
