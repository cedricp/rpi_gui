# UI Makefile
# 2016 Cedric PAILLE

###########################################################################################
#
# User defined variables
#
###########################################################################################

# If you're cross compiling, you likely want to change the line below
SYSROOT?=/usr

# If you're builing for a X86 on a X86 :)
SDL_ROOT_X86?=/sources/RTL/RADIO/INSTALL

# Be sure to define these variables
PYTHON_EXE?=`which python`
SWIG?=`which swig`

###########################################################################################

PYTHON_VERSION=${shell ${PYTHON_EXE} --version 2> /dev/stdout | cut -c 8-10}

ifndef BUILD
$(error Variable BUILD not specified use BUILD=X86 or BUILD=PI)
endif

SRC_ROOT_DIR=$(shell pwd)
PYTHON_INCLUDE?=$(SYSROOT)/include/python$(PYTHON_VERSION)

ifeq ($(BUILD),PI)
PLATFORM=RPI
LIB_FREETYPE2_LDFLAGS=-lfreetype
LIB_FREETYPE2_CXXFLAGS=-I$(SYSROOT)/include/freetype2

LIB_SDL2_LDFLAGS=-L$(SYSROOT)/lib -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic -lts
LIB_SDL2_CXXFLAGS=-I$(SYSROOT)/include

LIB_GLES2_LDFLAGS=-L$(SYSROOT)/opt/vc/lib -lm -lbcm_host -lvcos -lvchiq_arm -lGLESv2 -lEGL 
LIB_GLES2_CXXFLAGS=-I$(SYSROOT)/opt/vc/include -I$(SYSROOT)/opt/vc/include/interface/vcos/pthreads/

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

INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD_$(PLATFORM)
INSTALL_RESOURCES_DIR=$(INSTALL_DIR)/resources

OBJECTS_DIR=$(INSTALL_DIR)/.objs
INSTALL_LIB_DIR=$(INSTALL_DIR)/lib
BIN_DIR=$(INSTALL_DIR)/bin

UI_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libui.a
UTILS_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libutils.a
HW_FM_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libhw_fm.a
HW_TDA7419_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libhw_tda7419.a
HW_LCD_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libhw_lcd.a
WIRINGPI_LIBRARY_NAME=$(INSTALL_LIB_DIR)/libwiringpi.a

# Globals
GLOBAL_CXX_FLAGS+=-fPIC -I$(PYTHON_INCLUDE) -D_REENTRANT -ggdb $(LIB_FREETYPE2_CXXFLAGS) -I$(SRC_ROOT_DIR)/utils -I$(SRC_ROOT_DIR)/hardware/fm_lib -I$(SRC_ROOT_DIR)/hardware/tda7419_lib
GLOBAL_LD_FLAGS+=-L$(INSTALL_LIB_DIR) $(LIB_FREETYPE2_LDFLAGS) -ldl -lpthread -lrt -lmad
APPS_LD_FLAGS=-lui -lutils -lhw_fm -lhw_tda7419 -lwiringpi -lutil

export

.PHONY: check make_paths utils ui tests clean hardware python
all: check make_paths utils hardware ui tests python

hardware:
	$(MAKE) -C hardware

ui: make_paths utils 
	$(MAKE) -C ui

utils: make_paths
	$(MAKE) -C utils

tests: make_paths utils ui
	$(MAKE) -C tests

python: utils hardware ui
	$(MAKE) -C python

clean:
	$(MAKE) -C ui clean
	$(MAKE) -C utils clean
	$(MAKE) -C tests clean
	$(MAKE) -C python clean

distclean: clean
	rm -rf $(INSTALL_DIR)

check:
	@echo -e "\033[0;31mFound SWIG : ${SWIG}"
	@echo "Python executable : ${PYTHON_EXE} version ${PYTHON_VERSION}"
	@echo "Python include dir : ${PYTHON_INCLUDE}" 
	@echo "System root : ${SYSROOT}"
	@echo "C Compiler : ${CC}"
	@echo -e "C++ Compiler : ${CXX}\033[0m"

make_paths:
	@test -d $(OBJECTS_DIR) || mkdir -p $(OBJECTS_DIR)
	@test -d $(INSTALL_LIB_DIR) || mkdir -p $(INSTALL_LIB_DIR)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@test -d $(INSTALL_RESOURCES_DIR)|| mkdir -p $(INSTALL_RESOURCES_DIR)
	@cp -urf   resources/* $(INSTALL_RESOURCES_DIR)
	