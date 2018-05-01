obj-m += ./lame_dd/encdev.o
obj-m += ./lame_dd/decdev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	# sudo modprobe -rf decdev
	# sudo modprobe -rf encdev
	# sudo modprobe encdev
	# sudo modprobe decdev
	sudo insmod ./lame_dd/encdev.ko
	sudo insmod ./lame_dd/decdev.ko
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod decdev
	sudo rmmod encdev
