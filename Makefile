
PROJECT_NAME := Nano
COIN_PATH    := 44'/165'
BIP32_KEY    := ed25519 seed
DEVICE_PORT  ?= /dev/ttyS3

COMPONENT_EXTRA_CLEAN := Nano.elf

include jolt_wallet/make/app.mk

