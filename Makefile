# UI Makefile
# 2016 Cedric PAILLE

###########################################################################################
#
# User defined variables
#
###########################################################################################

# The path to your cross gcc compiler directory
CROSSGCC_ROOT=/sources/RPI/RPI_DEV/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin
# Path to your raspbian distro directory
SYSROOT=/sources/RPI/RPI_DEV/deb_root
SDL_ROOT_X86=/sources/RTL/RADIO/INSTALL
SDL_ROOT_CROSSPI=/sources/RPI/RPI_DEV/INSTALL

###########################################################################################

ifndef BUILD
$(warning No BUILD type specified using x86 build x11)
PLATFORM=X86
endif

SRC_ROOT_DIR=$(shell pwd)
INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD

ifeq ($(BUILD),crossrpi)
PLATFORM=RPI
export PKG_CONFIG_PATH=$(SYSROOT)/usr/lib/pkgconfig
CC=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-gcc-4.8.3
CXX=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-g++
AR=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-ar
LD=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-ld
INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD_RPI
GLOBAL_CXX_FLAGS+=--sysroot=$(SYSROOT) -I$(SYSROOT)/opt/vc/include -I$(SYSROOT)/usr/include
GLOBAL_LD_FLAGS+=--sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -L$(SYSROOT)/usr/lib/arm-linux-gnueabihf
$(warning Building ARM code for raspberrypi)
endif

OBJECTS_DIR=$(INSTALL_DIR)/BUILD/.objs
INSTALL_LIB_DIR=$(INSTALL_DIR)/lib
BIN_DIR=$(INSTALL_DIR)/bin

# Raspberrypi libraries
LIB_GLES2_LDFLAGS=-L$(SYSROOT)/opt/vc/lib -lm -lbcm_host -lvcos -lvchiq_arm -lGLESv2 -lEGL 
LIB_GLES2_CXXFLAGS=-I$(SYSROOT)/opt/vc/include -I$(SYSROOT)/opt/vc/include/interface/vcos/pthreads/

ifeq ($(PLATFORM),X86)
LIB_GL_LDFLAGS=-lGL -lGLU
LIB_GL_CXXFLAGS=-I/usr/inlcude -DUSE_OPENGL=1
LIB_SDL2_LDFLAGS=-L$(SDL_ROOT_X86)/lib -Wl,-Bstatic -lSDL2 -lSDL2main -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I$(SDL_ROOT_X86)/include
else
LIB_GL_LDFLAGS=$(LIB_GLES2_LDFLAGS)
LIB_GL_CXXFLAGS=$(LIB_GLES2_CXXFLAGS) -DUSE_GLES2=1
LIB_SDL2_LDFLAGS=-L$(SDL_ROOT_CROSSPI)/lib -Wl,-Bstatic -lSDL2 -lSDL2main -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I$(SDL_ROOT_CROSSPI)/include
endif

LIB_FREETYPE2_LDFLAGS=-lfreetype
LIB_FREETYPE2_CXXFLAGS=-I$(SYSROOT)/usr/include/freetype2

# Globals
GLOBAL_CXX_FLAGS+=-D_REENTRANT -ggdb $(LIB_FREETYPE2_CXXFLAGS)
GLOBAL_LD_FLAGS+=-L$(INSTALL_LIB_DIR) $(LIB_FREETYPE2_LDFLAGS) -ldl -lpthread -lrt

UI_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libui.a
UTILS_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libutils.a

export

.PHONY: make_paths utils ui tests clean
all: make_paths utils ui tests

ui: make_paths utils 
	$(MAKE) -C ui

utils: make_paths
	$(MAKE) -C utils

tests: make_paths utils ui
	$(MAKE) -C tests

clean:
	$(MAKE) -C ui clean
	$(MAKE) -C utils clean
	$(MAKE) -C tests clean

distclean:
	rm -rf $(INSTALL_DIR)

make_paths:
	@test -d $(OBJECTS_DIR) || mkdir -p $(OBJECTS_DIR)
	@test -d $(INSTALL_LIB_DIR) || mkdir -p $(INSTALL_LIB_DIR)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
