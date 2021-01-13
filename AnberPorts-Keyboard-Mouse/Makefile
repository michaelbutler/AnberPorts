CC = gcc
CCFLAGS = -W -Wall

BINARY = ../AnberPorts/oga_controls
LIBRARIES = -levdev
SOURCES = "main.c"

all:
	$(CC) $(CCFLAGS) $(INCLUDES) $(SOURCES) -o $(BINARY) $(LIBRARIES)

clean:
	rm -f $(BINARY)
