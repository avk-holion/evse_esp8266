#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := fase_once_esp


EXTRA_CFLAGS += -I$(CURDIR)/main/system
EXTRA_CFLAGS += -I$(CURDIR)/main/system/CMSIS_RTOS_V2
EXTRA_CFLAGS += -DESP_RTOS_SDK
EXTRA_CPPFLAGS += -I$(CURDIR)/main/system
EXTRA_CPPFLAGS += -I$(CURDIR)/main/system/modbus
EXTRA_CPPFLAGS += -I$(CURDIR)/main/system/CMSIS_RTOS_V2
EXTRA_COMPONENT_DIRS := $(CURDIR)/main/system $(CURDIR)/main/system/modbus $(CURDIR)/main/system/CMSIS_RTOS_V2
$(info EXTRA_CPPFLAGS: $(EXTRA_CPPFLAGS))
$(info EXTRA_COMPONENT_DIRS: $(EXTRA_COMPONENT_DIRS))

include $(IDF_PATH)/make/project.mk



