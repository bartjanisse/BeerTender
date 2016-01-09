# Makefile for gyroc

CC = arm-linux-gcc
CFLAGS = 
LDFLAGS = 
SOURCES = gyro.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = gyro

# Phony targets (i.e. not really files)
.PHONY: all clean

all: $(TARGET)

$(TARGET): gyro.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo gyro compiled successfully

install:
	scp gyro root@192.168.123.42:/bin

clean:
	@rm -rf $(TARGET) cgiprog.o
	@echo gyro is removed