CC = arm-linux-gcc
CFLAGS = -O2
LDFLAGS = 
HEADERS = 
SOURCES = testpid.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = testpid

.PHONY: all clean

all: $(TARGETS)
 
$(TARGETS) : $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

$(OBJECTS): $(HEADERS)

clean:
	@rm -rf $(TARGETS) $(OBJECTS)
