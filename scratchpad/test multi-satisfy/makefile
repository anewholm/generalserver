SHELL=/bin/sh

SOURCES_GENERAL=test.cpp

#-------------------------------- source and object files ------------------------------------------
CXX=g++
SOURCES=$(SOURCES_GENERAL)
OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=test

#-------------------------------- main ------------------------------------------
#linking $(OBJECTS) is the list of requirements before link will be attempted
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(ALLFLAGS) -o $(EXECUTABLE) $(INCLUDE_DIRS) $(LINKER_DIRS) $(OBJECTS) $(LINKER_FILES) 

#compile
.cpp.o:
	$(CXX) $(CPPFLAGS) $(INCLUDE_DIRS) -c $<
	
clear:
clean:
	rm -f $(OBJECTS)