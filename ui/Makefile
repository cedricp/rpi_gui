# UI library

###########################################################
# Variables Definitions
###########################################################

SOURCES = $(wildcard *.cpp)
SOURCES_C = $(wildcard fontlib/*.c)
OBJECTS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(SOURCES))
OBJECTS_C = $(patsubst fontlib/%.c, $(OBJECTS_DIR)/%.o, $(SOURCES_C))

DEPENDENCIES=$(OBJECTS:.o=.d) $(OBJECTS_C:.o=.d)
STATIC_LIB=$(UI_LIBRARY_NAME)

DEP_DOT_D=$(SOURCES:.cpp=.d) 
DEPENDENCY_FILES=$(addprefix $(OBJECTS_DIR)/,$(DEP_DOT_D))	

###########################################################
# Rules
###########################################################

all: $(STATIC_LIB)
	echo "Builing static library " $(STATIC_LIB)
	
CXXFLAGS+=-I$(SRC_ROOT_DIR)/utils $(GLOBAL_CXX_FLAGS) $(LIB_SDL2_CXXFLAGS) -DUSE_OPENGL=1
LD_FLAGS+=$(GLOBAL_LD_FLAGS) $(LIB_SDL2_LDFLAGS) $(LIB_GL_LDFLAGS)


$(OBJECTS_DIR)/%.o: %.cpp 
	$(CXX) $< -c $(CXXFLAGS) -o $@ 

$(OBJECTS_DIR)/%.o: fontlib/%.c
	$(CC) $< -c $(CXXFLAGS) -o $@ 

$(STATIC_LIB): $(OBJECTS) $(OBJECTS_C)
	@$(AR) rcs $@ $(OBJECTS) $(OBJECTS_C)
	
clean:
	@rm -rf $(OBJECTS) $(STATIC_LIB)

$(OBJECTS_DIR)/%.d: %.cpp
	@echo "Building dependencies for $<"
	@$(CXX) $< -MM $(CXXFLAGS) -MT $(OBJECTS_DIR)/$(<:.cpp=.o) -o $@


###########################################################
# Dependencies
###########################################################

-include $(DEPENDENCY_FILES)
