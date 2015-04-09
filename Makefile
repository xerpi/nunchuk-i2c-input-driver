obj-m = nunchuk-input-driver.o
KERNEL_HEADERS = /lib/modules/$(shell uname -r)/build

default:
	make -C $(KERNEL_HEADERS) M=$(PWD) modules

clean:
	make -C $(KERNEL_HEADERS) M=$(PWD) clean
	
insmod:
	insmod nunchuk-input-driver.ko

rmmod:
	rmmod nunchuk-input-driver.ko
