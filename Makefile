
PROJECT_NAME := Nano
COIN_PATH    := 44'/165'
BIP32_KEY    := ed25519 seed
DEVICE_PORT  ?= /dev/ttyS3

APP_VERSION_MAJOR := 0
APP_VERSION_MINOR := 0
APP_VERSION_PATCH := 0

include jolt_wallet/make/app.mk

