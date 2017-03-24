# Utils library

###########################################################
# Variables Definitions
###########################################################

SOURCES=$(wildcard *.cpp) 
SOURCES_C=$(wildcard *.c)
SOURCES_C+=$(wildcard vt100-emu/*.c)
OBJECTS=$(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(SOURCES))
OBJECTS+=$(patsubst %.c, $(OBJECTS_DIR)/%.o, $(SOURCES_C))
STATIC_LIB=$(UTILS_LIBRARY_NAME)

DEP_DOT_D=$(SOURCES:.cpp=.d)
DEP_DOT_D+=$(SOURCES_C:.c=.d)
DEPENDENCY_FILES=$(addprefix $(OBJECTS_DIR)/, $(DEP_DOT_D))	

CXXFLAGS+=$(GLOBAL_CXX_FLAGS) -I../include
LD_FLAGS+=

###########################################################
# Rules
###########################################################

all: $(DEPENDENCY_FILES) $(STATIC_LIB)
	echo "Builing static library " $(STATIC_LIB)
	
$(OBJECTS_DIR)/%.o: %.cpp
	@$(CXX) $< -c $(CXXFLAGS) -o $@
	
$(OBJECTS_DIR)/%.o: %.c
	@$(CC) $< -c $(CXXFLAGS) -o $@  
    
$(STATIC_LIB): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)
	
clean:
	@rm -rf $(OBJECTS) $(STATIC_LIB) $(DEPENDENCY_FILES)
	
$(OBJECTS_DIR)/%.d: %.cpp
	@echo "Building dependencies for $<"
	@$(CXX) $< -MM $(CXXFLAGS) -MT $(OBJECTS_DIR)/$(<:.cpp=.o) -o $@	

$(OBJECTS_DIR)/%.d: %.c
	@echo "Building dependencies for $<"
	@test -d $(OBJECTS_DIR)/vt100-emu || mkdir $(OBJECTS_DIR)/vt100-emu
	@$(CC) $< -MM $(CXXFLAGS) -MT $(OBJECTS_DIR)/$(<:.c=.o) -o $@	

###########################################################
# Dependencies check
###########################################################

-include $(DEPENDENCY_FILES)
