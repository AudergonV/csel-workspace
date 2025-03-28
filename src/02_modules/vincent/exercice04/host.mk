ifneq ($(KERNELRELEASE),)
obj-m += ex4.o		## name of the generated module

mymodule-objs := ex4.o	## list of objects needed for that module

else
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
 
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
 
endif
