# Out-of-tree build of hello_drv.ko
# Usage:  make          (builds against the running kernel)
#         make clean
#         make KDIR=/path/to/linux   (builds against another kernel tree)

obj-m := hello_drv.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
