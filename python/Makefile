SWIGFILES=*.swig
MODULE=$(INSTALL_DIR)/python/_ui.so
MODULEHW=$(INSTALL_DIR)/python/_hw.so
INCLUDE_DIR=../include
LIB_DIR=$(INSTALL_DIR)/lib

all: $(MODULEHW) $(MODULE)

%.c: %.i $(SWIGFILES)
	@test -d $(INSTALL_DIR)/python || mkdir  $(INSTALL_DIR)/python
	@echo "Swig generation"
	@swig -python -c++ -o $@ $<

$(MODULE): ui.c $(INSTALL_DIR)/lib/libui.a $(INSTALL_DIR)/lib/libutils.a
	@test -d $(INSTALL_DIR)/python || mkdir  $(INSTALL_DIR)/python
	@echo "Compiling UI Python module $@"
	@cp ui.py $(INSTALL_DIR)/python
	@$(CXX) -shared -fPIC -I$(INCLUDE_DIR) -L$(LIB_DIR) ui.c -o $(MODULE) $(GLOBAL_CXX_FLAGS) $(GLOBAL_LD_FLAGS) \
	 $(APPS_LD_FLAGS) $(LIB_GL_LDFLAGS) $(LIB_SDL2_LDFLAGS)

$(MODULEHW): hw.c $(INSTALL_DIR)/lib/libhw_fm.a $(INSTALL_DIR)/lib/libhw_tda7419.a
	@test -d $(INSTALL_DIR)/python || mkdir  $(INSTALL_DIR)/python
	@echo "Compiling HW Python module $@"
	@cp hw.py $(INSTALL_DIR)/python
	@$(CXX) -shared -fPIC -I$(INCLUDE_DIR) -L$(LIB_DIR) hw.c -o $(MODULEHW) $(GLOBAL_CXX_FLAGS) $(GLOBAL_LD_FLAGS) $(APPS_LD_FLAGS)
