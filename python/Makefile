SWIGFILES=$(wildcard *.swig)
MODULE=$(INSTALL_DIR)/python/_ui.so
MODULEHW=$(INSTALL_DIR)/python/_hw.so
INCLUDE_DIR=../include
LIB_DIR=$(INSTALL_DIR)/lib
INCFILES=$(wildcard ../include/*.h)

all: $(MODULEHW) $(MODULE)

%.c: %.i $(INCFILES)
	@test -d $(INSTALL_DIR)/python || mkdir  $(INSTALL_DIR)/python
	@echo "Swig generation $<"
	$(SWIG) -python -c++ -o $@ $<

ui.c: $(SWIGFILES)
hw.c: $(SWIGFILES)

$(MODULE): create_dir ui.c $(INSTALL_DIR)/lib/libui.a $(INSTALL_DIR)/lib/libutils.a
	@echo "Compiling UI Python module $@"
	@cp ui.py $(INSTALL_DIR)/python
	@$(CXX) -shared -fPIC -I$(INCLUDE_DIR) -L$(LIB_DIR) ui.c -o $(MODULE) $(GLOBAL_CXX_FLAGS) $(GLOBAL_LD_FLAGS) \
	 $(APPS_LD_FLAGS) $(LIB_GL_LDFLAGS) $(LIB_SDL2_LDFLAGS)

$(MODULEHW): create_dir hw.c $(INSTALL_DIR)/lib/libhw_fm.a $(INSTALL_DIR)/lib/libhw_tda7419.a
	@echo "Compiling HW Python module $@"
	@cp hw.py $(INSTALL_DIR)/python
	@$(CXX) -shared -fPIC -I$(INCLUDE_DIR) -L$(LIB_DIR) hw.c -o $(MODULEHW) $(GLOBAL_CXX_FLAGS) $(GLOBAL_LD_FLAGS) $(APPS_LD_FLAGS)

create_dir: 
	@test -d $(INSTALL_DIR)/python || mkdir  $(INSTALL_DIR)/python
	
clean:
	@rm -f hw.c hw.h hw.py ui.c ui.h ui.py
	@rm $(MODULEHW) $(MODULE)