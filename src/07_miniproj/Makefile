all: daemon cli module

daemon:
	$(MAKE) -C daemon

cli:
	$(MAKE) -C cli

module:
	$(MAKE) -C module

clean:
	$(MAKE) -C daemon clean
	$(MAKE) -C cli clean
	$(MAKE) -C module clean

install:
	$(MAKE) -C daemon install
	$(MAKE) -C cli install
	$(MAKE) -C module install

uninstall:
	$(MAKE) -C daemon uninstall
	$(MAKE) -C cli uninstall
	$(MAKE) -C module uninstall

.PHONY: all daemon cli clean module install uninstall