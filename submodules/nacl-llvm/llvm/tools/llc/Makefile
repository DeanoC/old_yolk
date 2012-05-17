#===- tools/llc/Makefile -----------------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##

LEVEL := ../..
TOOLNAME := llc
LINK_COMPONENTS := all-targets bitreader asmparser

# @LOCALMOD-BEGIN
# Link in libelf.a (unless we're building the JIT translator)
ifeq ($(NACL_SB_JIT),0)
  ExtraLibs += -lelf
endif
# @LOCALMOD-END

include $(LEVEL)/Makefile.common

