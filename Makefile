#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Nano
EXTRA_COMPONENT_DIRS = $(PROJECT_PATH)/jolt_wallet/components $(PROJECT_PATH)/jolt_wallet $(PROJECT_PATH)/src
include $(IDF_PATH)/make/project.mk
