# PSP Software Development Kit - http://www.pspdev.org
# -----------------------------------------------------------------------
# Licensed under the BSD license, see LICENSE in PSPSDK root for details.
#
# build.mak - Base makefile for projects using PSPSDK.
#
# Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
# Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
# Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
#
# $Id: build.mak 1734 2006-01-22 12:22:34Z tyranid $

# Note: The PSPSDK make variable must be defined before this file is included.
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
endif
BINARIES = $(PSPDEV)/bin
CC       = $(BINARIES)/psp-gcc
CXX      = $(BINARIES)/psp-g++
AS       = $(BINARIES)/psp-gcc
LD       = $(BINARIES)/psp-gcc
AR       = $(BINARIES)/psp-ar
RANLIB   = $(BINARIES)/psp-ranlib
STRIP    = $(BINARIES)/psp-strip
FIXUP    = $(BINARIES)/psp-fixup-imports

# Add in PSPSDK includes and libraries.
INCDIR   := $(INCDIR) ./ $(PSPSDK)/include
LIBDIR   := $(LIBDIR) ./ $(PSPSDK)/lib

CFLAGS   := $(addprefix -I,$(INCDIR)) $(addprefix -L,$(LIBDIR)) $(CFLAGS)

ifeq ($(USE_KERNEL_LIBC),1)
# Use the PSP's kernel libc
PSPSDK_LIBC_LIB = 
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
ifeq ($(USE_PSPSDK_LIBC),1)
# Use the pspsdk libc
PSPSDK_LIBC_LIB = -lpsplibc
CFLAGS := -I$(PSPSDK)/include/libc $(CFLAGS)
else
# Use newlib (urgh)
PSPSDK_LIBC_LIB = -lc
endif
endif

# Link with following default libraries.  Other libraries should be specified in the $(LIBS) variable.
ifeq ($(USE_KERNEL_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay_driver -lpspctrl_driver -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspkernel
else
ifeq ($(USE_USER_LIBS),1)
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser
else
PSPSDK_LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspsdk
LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspnet \
			-lpspnet_inet -lpspnet_apctl -lpspnet_resolver -lpsputility \
			-lpspuser -lpspkernel
endif
endif

ifndef PSP_EBOOT
PSP_EBOOT = EBOOT.PBP
endif

all: RemObj $(EXTRA_TARGETS) $(FINAL_TARGET)

$(TARGET).elf: $(OBJS) $(EXPORT_OBJ)
	$(LINK.c) $^ $(LIBS) -o $@
	$(FIXUP) $@

RemObj:
	-C:\pspdev\bin\rm -f startup.o

$(PSP_EBOOT): $(TARGET).elf
	$(STRIP) -R .sceStub.text -R .pdr -R .sbss -R .mdebug.eabi32 -R .gcc_compiled_long32 -R .comment -s -X -x -g --strip-unneeded $(TARGET).elf
	-$(BINARIES)/rm -f __SCE__$(TARGET)\EBOOT.PBP
	-$(BINARIES)/rm -f %__SCE__$(TARGET)\EBOOT.PBP
	-$(BINARIES)/rmdir __SCE__$(TARGET)
	-$(BINARIES)/rmdir %__SCE__$(TARGET)
	-$(BINARIES)/mkdir __SCE__$(TARGET)
	-$(BINARIES)/mkdir %__SCE__$(TARGET)
	-$(BINARIES)/mv $(TARGET).elf __SCE__$(TARGET)\EBOOT.PBP
	-Tools\mksfo $(TARGET) SPARAM.SFO
	-Tools\pack-pbp %__SCE__$(TARGET)\EBOOT.PBP SPARAM.SFO
#	-$(BINARIES)/psp-packer __SCE__$(TARGET)\EBOOT.PBP
	-$(BINARIES)/rm SPARAM.SFO startup.o

clean: 
	-C:\pspdev\bin\rm -f $(FINAL_TARGET) $(EXTRA_CLEAN) $(OBJS) $(PSP_EBOOT) $(EXTRA_TARGETS)
