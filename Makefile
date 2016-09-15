# UI Makefile
# 2016 Cedric PAILLE

SRC_ROOT_DIR=$(shell pwd)
OBJECTS_DIR=$(SRC_ROOT_DIR)/BUILD/.objs
INSTALL_DIR=$(SRC_ROOT_DIR)/BUILD
INSTALL_LIB_DIR=$(INSTALL_DIR)/lib
BIN_DIR=$(INSTALL_DIR)/bin

LIB_GL_LDFLAGS=-lGL -lGLU
LIB_SDL2_LDFLAGS=-L/sources/RTL/RADIO/INSTALL/lib -Wl,-Bstatic -lSDL2 -lSDL2main -Wl,-Bdynamic
LIB_SDL2_CXXFLAGS=-I/sources/RTL/RADIO/INSTALL/include

GLOBAL_CXX_FLAGS=-D_REENTRANT -ggdb -I/usr/include/freetype2
GLOBAL_LD_FLAGS=-L$(INSTALL_LIB_DIR) -lfreetype

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
