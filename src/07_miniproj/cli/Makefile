CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = fanctl
SOURCES = fanctl.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /rootfs/usr/bin/

uninstall:
	rm -f /rootfs/usr/bin/$(TARGET)

.PHONY: all clean install uninstall