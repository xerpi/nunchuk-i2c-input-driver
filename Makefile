obj-m = nunchuk-i2c.o
KERNEL_HEADERS = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNEL_HEADERS) M=$(PWD) modules

clean:
	make -C $(KERNEL_HEADERS) M=$(PWD) clean
	
insmod:
	insmod nunchuk-i2c.ko

rmmod:
	rmmod nunchuk-i2c.ko

new_device:
	echo nunchuk-i2c 0x52 > /sys/bus/i2c/devices/i2c-1/new_device

delete_device:
	echo 0x52 > /sys/bus/i2c/devices/i2c-1/delete_device
