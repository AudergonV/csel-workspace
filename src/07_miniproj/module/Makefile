# Part executed when called from kernel build system:
ifneq ($(KERNELRELEASE),)
obj-m += csel.o		## name of the generated module
csel-objs := \
    src/main.o \
    src/cpu_temperature.o \
    src/status_led.o \
    src/fan_control.o
# Part executed when called from standard make in module source directory:
else
include ../../buildroot_path
include ../../kernel_settings

PWD     := $(shell pwd)
SRC     := $(PWD)/src

all:
	$(MAKE) -C $(KDIR) \
	        M=$(PWD) \
	        ARCH=$(CPU) \
	        CROSS_COMPILE=$(TOOLS) \
	        modules

clean:
	$(MAKE) -C $(KDIR) \
	        M=$(PWD) \
	        clean

install:
	$(MAKE) -C $(KDIR) \
	        M=$(PWD) \
	        INSTALL_MOD_PATH=$(MODPATH) \
	        modules_install

uninstall:
	rm -f $(MODPATH)/lib/modules/$(KVER)/extra/csel.ko
	depmod -a -b $(MODPATH) $(KVER)

endif
