CONFIG_MODULE_SIG=ns
obj-m	+=	prodConsSemaphoreCs12b1030.o
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
