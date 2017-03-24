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

# If you're cross compiling, you likely want to change the 2 lines above
SYSROOT=/sources/RPI/RPI_DEV/deb_root
SDL_ROOT_CROSSPI=/sources/RPI/RPI_DEV/INSTALL
# If you're builing for a X86 on a X86 :)
SDL_ROOT_X86=/sources/RTL/RADIO/INSTALL
# If you're compiling from the PI
SDL_ROOT_PI=/home/cedric/DEV/ROOT_INSTALL

###########################################################################################

ifndef BUILD
$(error Variable BUILD not specified use BUILD=X86 or BUILD=CROSSPI)
endif

SRC_ROOT_DIR=$(shell pwd)

ifeq ($(BUILD),CROSSPI)
PLATFORM=RPI_CROSS
CC=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-gcc-4.8.3
CXX=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-g++
AR=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-ar
LD=$(CROSSGCC_ROOT)/arm-linux-gnueabihf-ld
INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD_RPI
GLOBAL_CXX_FLAGS+=--sysroot=$(SYSROOT) -I$(SYSROOT)/opt/vc/include -I$(SYSROOT)/usr/include -g
GLOBAL_LD_FLAGS+=--sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -L$(SYSROOT)/usr/lib/arm-linux-gnueabihf

LIB_SDL2_LDFLAGS=-L$(SDL_ROOT_CROSSPI)/lib -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I$(SDL_ROOT_CROSSPI)/include

LIB_GLES2_LDFLAGS=-L$(SYSROOT)/opt/vc/lib -lm -lbcm_host -lvcos -lvchiq_arm -lGLESv2 -lEGL 
LIB_GLES2_CXXFLAGS=-I$(SYSROOT)/opt/vc/include -I$(SYSROOT)/opt/vc/include/interface/vcos/pthreads/

LIB_GL_LDFLAGS=$(LIB_GLES2_LDFLAGS)
LIB_GL_CXXFLAGS=$(LIB_GLES2_CXXFLAGS) -DUSE_GLES2=1

LIB_FREETYPE2_LDFLAGS=-lfreetype
LIB_FREETYPE2_CXXFLAGS=-I$(SYSROOT)/usr/include/freetype2
$(warning Building ARM cross compilation code for raspberrypi EGL - GLES2)
else
ifeq ($(BUILD),PI)
PLATFORM=RPI
LIB_FREETYPE2_LDFLAGS=-lfreetype
LIB_FREETYPE2_CXXFLAGS=-I$(SYSROOT)/usr/include/freetype2

LIB_SDL2_LDFLAGS=-L$(SDL_ROOT_PI)/lib -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I$(SDL_ROOT_PI)/include

LIB_GLES2_LDFLAGS=-L$(SDL_ROOT_PI)/opt/vc/lib -lm -lbcm_host -lvcos -lvchiq_arm -lGLESv2 -lEGL 
LIB_GLES2_CXXFLAGS=-I$(SDL_ROOT_PI)/opt/vc/include -I$(SDL_ROOT_PI)/opt/vc/include/interface/vcos/pthreads/

LIB_GL_LDFLAGS=$(LIB_GLES2_LDFLAGS)
LIB_GL_CXXFLAGS=$(LIB_GLES2_CXXFLAGS) -DUSE_GLES2=1
$(warning Building ARM code for raspberrypi EGL - GLES2)
else
ifeq ($(BUILD),X86)
LIB_FREETYPE2_LDFLAGS=-lfreetype
LIB_FREETYPE2_CXXFLAGS=-I/usr/include/freetype2

PLATFORM=X86
LIB_SDL2_LDFLAGS=-L$(SDL_ROOT_X86)/lib -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I$(SDL_ROOT_X86)/include

LIB_GL_LDFLAGS=-lGL -lGLU
LIB_GL_CXXFLAGS=-I/usr/inlcude -DUSE_OPENGL=1
$(warning Building X86 code X11 GLX)
endif
endif
endif

INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD_$(PLATFORM)
INSTALL_RESOURCES_DIR=$(INSTALL_DIR)/resources

OBJECTS_DIR=$(INSTALL_DIR)/.objs
INSTALL_LIB_DIR=$(INSTALL_DIR)/lib
BIN_DIR=$(INSTALL_DIR)/bin

UI_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libui.a
UTILS_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libutils.a
HW_FM_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libhw_fm.a
HW_TDA7419_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libhw_tda7419.a
WIRINGPI_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libwiringpi.a

# Globals
GLOBAL_CXX_FLAGS+=-D_REENTRANT -ggdb $(LIB_FREETYPE2_CXXFLAGS) -I$(SRC_ROOT_DIR)/utils -I$(SRC_ROOT_DIR)/hardware/fm_lib -I$(SRC_ROOT_DIR)/hardware/tda7419_lib
GLOBAL_LD_FLAGS+=-L$(INSTALL_LIB_DIR) $(LIB_FREETYPE2_LDFLAGS) -ldl -lpthread -lrt
APPS_LD_FLAGS=-lui -lutils -lhw_fm -lhw_tda7419 -lwiringpi -lutil

export

.PHONY: make_paths utils ui tests clean hardware
all: make_paths utils hardware ui tests 

hardware:
	$(MAKE) -C hardware

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
	@test -d $(INSTALL_RESOURCES_DIR)|| mkdir -p $(INSTALL_RESOURCES_DIR)
	@cp -urf   resources/* $(INSTALL_RESOURCES_DIR)
