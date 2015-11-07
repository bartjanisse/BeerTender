# 
#	Generic Makefile for simple projects.
#
#	(C) 2015, A.W. Janisse
#
#	Macro's:
#		OUTPUT 	: Name of the executable
#		PIDIR   : Place for installation on the Raspberry PI
#		CC 		: Default compiler. (Note make pi will build for the Raspberry Pi platform)
#		LIBS 	: Libraries to use when building
#		CFLAGS 	: Compiler flags
#		ZIPDIR  : Directory to put the backup archive files
#

OUTPUT 	= helloi2c
SUBDIRS = ../common
PIDIR   = root@192.168.123.42:/bin
CC 		= gcc
LIBS 	= 
CFLAGS 	= -O2 -Wall -Werror
ZIPDIR  = ../backup

### -----[ Do not change anything below this line ]----- ###

# Remove any unwanted leading and trailing spaces
TARGET = $(strip $(OUTPUT))
# Retreive a list of source files (ending with .c)
SOURCES  = $(wildcard *.c) 
SOURCES += $(foreach sdir,$(SUBDIRS),$(wildcard $(sdir)/*.c))
# Replace all .c in the sources list to .o
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))
# Retreive a list of header files (ending with .h)
HEADERS  = $(wildcard *.h)
HEADERS += $(foreach sdir,$(SUBDIRS),$(wildcard $(sdir)/*.h))
# Build the archive name and set extension
TARNAME = $(ZIPDIR)/$(TARGET)_$(shell date +'%Y%m%d_%H%M')$(TAREXT)
TAREXT  = .tar

.PHONY: all debug clean install info html pdf backup pi

# Rule to perform when just make is executed.
all: $(TARGET)

# implicit rule for building the object files.
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Don't delete intermediate files when this make is aborted for some reason.
.PRECIOUS: $(TARGET) $(OBJECTS)

# Here the compiling hapens	
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

# Build all with debug information. The X86 toolchain is used.
debug: CFLAGS += -g
debug: all

# Build all with the arm-linux-gcc toolchain.
pi: CC=arm-linux-gcc
pi: all

# Just cleanup by removing the exectable en .obj files.
clean:
	@rm -rf $(TARGET) $(OBJECTS)

# Give information about this Makefile.
info: 
	@echo ==================================================================
	@echo Output"   ": $(TARGET)
	@echo Sources"  ": $(SOURCES)
	@echo Headers"  ": $(HEADERS)
	@echo Subdirs"  ": $(SUBDIRS)
	@echo Objects"  ": $(OBJECTS)
	@echo Libraries: $(LIBS)
	@echo Compiler : $(CC)
	@echo CFlags"   ": $(CFLAGS)
	@echo Zip dir"  ": $(ZIPDIR)
	@echo ==================================================================
	
# Copy the executable over to the Raspberry PI.
install:
	@echo Connecting...
	@scp $(TARGET) $(PIDIR)
	@echo installation done!

# Create an archive file containing all the essential files for reproduction.
backup: clean
backup:
	@mkdir -p $(ZIPDIR)
	@tar -cf $(TARNAME) .
	@echo Created archive: $(TARNAME)

# Produce the HTML documentation based on the settings in Doxyfile.
html:
	@doxygen ../Doxyfile
	@echo HTML documentation is generated in doc/html

