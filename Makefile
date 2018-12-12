#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Nano

EXTRA_COMPONENT_DIRS = \
        $(PROJECT_PATH)/src \
        $(PROJECT_PATH)/jolt_wallet/jolt_os/lvgl \
        $(PROJECT_PATH)/jolt_wallet/jolt_os \
        $(PROJECT_PATH)/jolt_wallet/components

CFLAGS += \
        -DJOLT_GUI_DEBUG_FUNCTIONS

include $(IDF_PATH)/make/project.mk
