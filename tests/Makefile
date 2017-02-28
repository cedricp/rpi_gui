# UI library

SOURCES = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp, $(BIN_DIR)/%, $(SOURCES))

all: $(PROGS)
	echo "Builing programs " $(PROGS)
	
CXXFLAGS+=-I$(SRC_ROOT_DIR)/utils -I$(SRC_ROOT_DIR)/include $(GLOBAL_CXX_FLAGS) $(LIB_SDL2_CXXFLAGS) $(LIB_GL_CXXFLAGS)
LD_FLAGS+=$(GLOBAL_LD_FLAGS) $(APPS_LD_FLAGS) $(LIB_SDL2_LDFLAGS) $(LIB_GL_LDFLAGS)

$(BIN_DIR)/%: %.cpp $(UI_LIBRARY_NAME) $(UTILS_LIBRARY_NAME)
	$(CXX) $< $(LD_FLAGS) $(CXXFLAGS) -o $@ 

	
clean:
	@rm -rf $(OBJECTS) $(PROGS)
