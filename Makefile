obj-m += chardev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo modprobe -rf chardev
	sudo modprobe chardev
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
