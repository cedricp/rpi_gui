# UI Makefile
# 2016 Cedric PAILLE

ifndef BUILD
$(warning No BUILD type specified using x86 build x11)
PLATFORM=X86
endif

ifeq ($(BUILD),rpi)
PLATFORM=RPI
endif

SRC_ROOT_DIR=$(shell pwd)
OBJECTS_DIR=$(SRC_ROOT_DIR)/BUILD/.objs
INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD
INSTALL_LIB_DIR=$(INSTALL_DIR)/lib
BIN_DIR=$(INSTALL_DIR)/bin

# Raspberrypi libraries
LIB_GLES2_LDFLAGS=-L/opt/vc/lib -lGLESv2 -lEGL -lm -lbcm_host
LIB_GLES2_CXXFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads/

ifeq ($(PLATFORM),X86)
LIB_GL_LDFLAGS=-lGL -lGLU
LIB_GL_CXXFLAGS=-I/usr/inlcude -DUSE_OPENGL=1
else
LIB_GL_LDFLAGS=$(LIB_GLES2_LDFLAGS)
LIB_GL_CXXFLAGS=$(LIB_GLES2_CXXFLAGS) -DUSE_GLES2=1
endif

# SDL2 libraries
LIB_SDL2_LDFLAGS=-L/sources/RTL/RADIO/INSTALL/lib -Wl,-Bstatic -lSDL2 -lSDL2main -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I/sources/RTL/RADIO/INSTALL/include

# Globals
GLOBAL_CXX_FLAGS=-D_REENTRANT -ggdb `pkg-config freetype2 --cflags`
GLOBAL_LD_FLAGS=-L$(INSTALL_LIB_DIR) `pkg-config freetype2 --libs` -ldl -lpthread -lrt

UI_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libui.a
UTILS_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libutils.a

export

.PHONY: build_paths utils ui tests clean
all: build_paths utils ui tests

ui: utils
	$(MAKE) -C ui

utils:
	$(MAKE) -C utils

tests:
	$(MAKE) -C tests

clean:
	$(MAKE) -C ui clean
	$(MAKE) -C utils clean
	$(MAKE) -C tests clean

distclean:
	rm -rf $(INSTALL_DIR)

build_paths:
	@test -d $(OBJECTS_DIR) || mkdir -p $(OBJECTS_DIR)
	@test -d $(INSTALL_LIB_DIR) || mkdir -p $(INSTALL_LIB_DIR)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
