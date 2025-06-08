all: daemon cli

daemon:
	$(MAKE) -C daemon

cli:
	$(MAKE) -C cli

clean:
	$(MAKE) -C daemon clean
	$(MAKE) -C cli clean

install:
	$(MAKE) -C daemon install
	$(MAKE) -C cli install

uninstall:
	$(MAKE) -C daemon uninstall
	$(MAKE) -C cli uninstall

.PHONY: all daemon cli clean install uninstall