# Utils library

###########################################################
# Variables Definitions
###########################################################

SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(SOURCES))
STATIC_LIB=$(HW_FM_LIBRARY_NAME)

CXXFLAGS+=$(GLOBAL_CXX_FLAGS) -I../../utils
LD_FLAGS+=

###########################################################
# Rules
###########################################################

all: $(STATIC_LIB)
	echo "Builing static library " $(STATIC_LIB)
	
$(OBJECTS_DIR)/%.o: %.cpp
	@$(CXX) $< -c $(CXXFLAGS) -o $@ 
    
$(STATIC_LIB): $(OBJECTS)
	@$(AR) rcs $@ $(OBJECTS)
	
clean:
	@rm -rf $(OBJECTS) $(STATIC_LIB) $(DEPENDENCY_FILES)
	
$(OBJECTS_DIR)/%.d: %.cpp
	@echo "Building dependencies for $<"
	@$(CXX) $< -MM $(CXXFLAGS) -MT $(OBJECTS_DIR)/$(<:.cpp=.o) -o $@	

