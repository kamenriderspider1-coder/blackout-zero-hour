.SUFFIXES:
.PHONY: all clean
TARGET := blackout-zero-hour
BUILD := build
SOURCES := source
DATA :=
INCLUDES :=
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS := -g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH) $(INCLUDES)
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS := $(ARCH)
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -Wl,-Map,$(notdir $*.map)
LIBS := -lnx
include $(DEVKITPRO)/libnx/switch_rules
