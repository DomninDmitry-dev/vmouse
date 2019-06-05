KDIR = /lib/modules/$(shell uname -r)/build
CCFLAGS = -C
PWD = $(shell pwd)
NAME = vmouse

obj-m   := $(NAME).o 

all: test
	$(MAKE) $(CCFLAGS) $(KDIR) M=$(PWD) modules

test: test.c
	gcc test.c -o test
	
clean: 
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order 
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp* 
	@rm -rf .tmp_versions
	@rm -f *.ko *.symvers
	
load:
	sudo insmod $(NAME).ko
unload:
	sudo rmmod $(NAME).ko