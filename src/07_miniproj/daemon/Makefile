EXEC=fanmgr
SRCS=$(wildcard *.c) $(wildcard gpio/*.c) $(wildcard oled/*.c)

target=nano

TOOLCHAIN_PATH=/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)aarch64-linux-
CFLAGS=-Wall -Wextra -g -c -O0 -MD -std=gnu11 -Igpio -Ioled
CFLAGS+=-mcpu=cortex-a53 -funwind-tables
OBJDIR=.obj/nano

CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip

OBJDIR=.obj/$(target)
OBJS= $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

all: $(EXEC)
	echo $(EXEC)
	echo $(SRCS)

$(EXEC): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

install: $(EXEC)
	install -m 755 $(EXEC) /rootfs/usr/bin/

uninstall:
	rm -f /rootfs/usr/bin/$(EXEC)

clean:
	rm -Rf $(OBJDIR) $(EXEC) $(EXEC)_s .obj *~

clean_all: clean

-include $(OBJS:.o=.d)

.PHONY: all clean clean_all install uninstall
