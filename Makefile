#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Nano
all: jolt_app.elf

# Directories to build: esp-idf, components, main
# Directories to -I esp-idf, components, main, jolt_wallet
# Directoreis to link: main, components

EXTRA_COMPONENT_DIRS = $(PROJECT_PATH)/jolt_wallet/components $(PROJECT_PATH)/src
#$(PROJECT_PATH)/jolt_wallet/main

#JOLT_APP_COMPONENT_DIRS = $(wildcard $(PROJECT_PATH)/components/*/component.mk
#COMPONENTS := $(dir $(foreach cd,$(COMPONENT_DIRS),$(wildcard $(cd)/*/component.mk) $(wildcard $(cd)/component.mk) ))
#COMPONENTS := $(sort $(foreach comp,$(COMPONENTS),$(lastword $(subst /, ,$(comp)))))

#ELF_LINK
ELF_LDFLAGS = -Wl,-r -nostartfiles -nodefaultlibs -nostdlib -Os \
		  -Wl,-Tld/esp32.rom.nanofmt.ld \
		  -Wl,-Tld/esp32.rom.ld \
		   -Wl,--unresolved-symbols=ignore-in-object-files


#COMPONENT_DIRS := $(PROJECT_PATH)/components $(EXTRA_COMPONENT_DIRS) $(IDF_PATH)/components $(PROJECT_PATH)/main
include project.mk

# Goals: only link components in the "components" directory
# depends on all of those .a
#
#$(error $(foreach libcomp,$(COMPONENT_LIBRARIES),$(BUILD_DIR_BASE)/$(libcomp)/lib$(libcomp).a))
#$(error $(COMPONENT_LINKER_DEPS) )
#$(error $(COMPONENT_PROJECT_VARS) )

$(APP_ELF):;

jolt_app.elf: $(foreach libcomp,$(COMPONENT_LIBRARIES),$(BUILD_DIR_BASE)/$(libcomp)/lib$(libcomp).a) $(COMPONENT_LINKER_DEPS) $(COMPONENT_PROJECT_VARS)
	$(summary) LD $(patsubst $(PWD)/%,%,$@)
	xtensa-esp32-elf-gcc $(ELF_LDFLAGS) -o $@ /Users/brianpugh/esp/nano_app/build/src/libsrc.a

