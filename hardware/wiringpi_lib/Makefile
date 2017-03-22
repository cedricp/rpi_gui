# WiringPI library

###########################################################
# Variables Definitions
###########################################################

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c, $(OBJECTS_DIR)/%.o, $(SOURCES))
STATIC_LIB=$(WIRINGPI_LIBRARY_NAME)

CXXFLAGS+=$(GLOBAL_CXX_FLAGS) -I../include
LD_FLAGS+=

###########################################################
# Rules
###########################################################

all: $(STATIC_LIB)
	echo "Builing static library " $(STATIC_LIB)
	
$(OBJECTS_DIR)/%.o: %.c
	@$(CC) $< -c $(CXXFLAGS) -o $@ 
    
$(STATIC_LIB): $(OBJECTS)
	@$(AR) rcs $@ $(OBJECTS)
	
clean:
	@rm -rf $(OBJECTS) $(STATIC_LIB) $(DEPENDENCY_FILES)
	
$(OBJECTS_DIR)/%.d: %.c
	@echo "Building dependencies for $<"
	@$(CC) $< -MM $(CXXFLAGS) -MT $(OBJECTS_DIR)/$(<:.cpp=.o) -o $@	

