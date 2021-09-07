TARGET = AT3Replay
OBJS = AT3.o startup.o

INCDIR =
CFLAGS = -G0 -Wall -fmerge-all-constants -ffunction-sections -fdata-sections -nostartfiles -nostdlib
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS = -nostartfiles -nostdlib -ffunction-sections -fdata-sections
LIBS= -lpspsmall_gum -lpspsmall_gu -lpsppower -lvfpumath -lpspaudio -lpspaudiocodec

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = $(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
PSPDEV=$(shell psp-config --pspdev-path)
include ./build.mak
