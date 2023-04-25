BASE_NAME = cpui-drv
PATH_TO_KERNEL_SRC := /lib/modules/$(shell uname -r)/build
obj-m += $(BASE_NAME).o

all:
	$(MAKE) -C $(PATH_TO_KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(PATH_TO_KERNEL_SRC) M=$(PWD) clean
