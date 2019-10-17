#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

# App Specific Parameters
APP_NAME     := Nano
COIN_PATH    := 44'/165'
BIP32_KEY    := ed25519 seed
ELF2JELF     := jolt_wallet/elf2jelf/elf2jelf.py
PYTHONBIN    ?= python3
DEVICE_PORT  := /dev/ttyS3

# Convert names to absolute paths and stuff
ELF_BIN_NAME             := $(APP_NAME).elf
JELF_BIN_NAME            := $(APP_NAME).jelf

ELF_BIN_NAME             := $(abspath $(ELF_BIN_NAME))
JELF_BIN_NAME            := $(abspath $(JELF_BIN_NAME))
JELF_BIN_COMPRESSED_NAME := $(abspath $(JELF_BIN_COMPRESSED_NAME))
ELF2JELF                 := $(abspath $(ELF2JELF))

PROJECT_NAME := Nano

EXTRA_COMPONENT_DIRS = \
		$(IDF_PATH)/tools/unit-test-app/components/ \
        $(PROJECT_PATH)/src \
        $(PROJECT_PATH)/jolt_wallet/jolt_os/lvgl \
        $(PROJECT_PATH)/jolt_wallet/jolt_os \
        $(PROJECT_PATH)/jolt_wallet/components


CFLAGS += \
        -Werror \
        -DJOLT_APP \
        -DJOLT_GUI_DEBUG_FUNCTIONS


include $(IDF_PATH)/make/project.mk


.PHONY: jflash

$(ELF_BIN_NAME): app
	xtensa-esp32-elf-gcc -Wl,-static -nostartfiles -nodefaultlibs -nostdlib -Os \
    		-ffunction-sections -fdata-sections -Wl,--gc-sections \
    		-s -o $(ELF_BIN_NAME) \
    		-Wl,-r \
    		-Wl,-eapp_main \
    		-Wl,--warn-unresolved-symbols \
    		build/src/libsrc.a \
    		build/nano_parse/libnano_parse.a \
    		build/nano_lib/libnano_lib.a
	# Trim unnecessary sections
	xtensa-esp32-elf-objcopy --remove-section=.comment \
	         --remove-section=.xtensa.info \
	         --remove-section=.xt.prop \
	         --remove-section=.rela.xt.prop \
	         --remove-section=.xt.lit \
	         --remove-section=.rela.xt.lit \
	         --remove-section=.text \
	         $(ELF_BIN_NAME)


$(JELF_BIN_NAME): $(ELF_BIN_NAME)
	$(PYTHONBIN) $(ELF2JELF)  "$(ELF_BIN_NAME)" \
    		--output "$(JELF_BIN_NAME)" \
    		--coin "$(COIN_PATH)" \
    		--bip32key "$(BIP32_KEY)"

japp: $(JELF_BIN_NAME) ;


jflash: $(JELF_BIN_NAME)
	$(PYTHONBIN) jolt_wallet/pyutils/usb_upload.py --port $(DEVICE_PORT) $(JELF_BIN_NAME)

print-%  : ; @echo $* = $($*)
