CC = arm-linux-gcc
CFLAGS = -O2
#LDFLAGS = 
SOURCES = gyro.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = gyro

# Phony targets (i.e. not really files)
.PHONY: all clean

all: $(TARGETS)
 
$(TARGETS) : gyro.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo gyro is made

clean:
	@rm -rf $(TARGET) gyro.o
	@echo gyro is removed
