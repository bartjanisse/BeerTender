CC = arm-linux-gcc
CFLAGS = -O2
LDFLAGS = 
HEADERS = pwm_j.h
SOURCES = pwm_j.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = pwm_j

.PHONY: all clean

all: $(TARGETS)
 
$(TARGETS) : $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

$(OBJECTS): $(HEADERS)

clean:
	@rm -rf $(TARGETS) $(OBJECTS)
