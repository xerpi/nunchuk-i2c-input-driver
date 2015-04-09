#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>

static struct input_dev *input;

static int __init nunchuk_input_driver_init(void)
{
	int error;
	
	printk(KERN_INFO "nunchuk_input_driver_init()\n");
	
	input = input_allocate_device();
	if (!input) {
		printk(KERN_ERR "nunchuk_input_driver_init: "
			"not enough memory\n");
		error = -ENOMEM;
		goto err_nomem;
	}

	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_0, input->keybit);
	
	error = input_register_device(input);
	if (error) {
		printk(KERN_ERR "nunchuk_input_driver_init: "
			"failed to register input device (%d)\n", error);
		goto err_freedev;
	}
	
	return 0;

err_freedev:
	input_free_device(input);
err_nomem:
	return error;
}

static void __exit nunchuk_input_driver_exit(void)
{
	printk(KERN_INFO "nunchuk_input_driver_exit()\n");
	
	input_unregister_device(input);
}

module_init(nunchuk_input_driver_init);
module_exit(nunchuk_input_driver_exit);

MODULE_AUTHOR("Sergi Granell");
MODULE_DESCRIPTION("nunchuk input driver");
MODULE_LICENSE("GPL");
