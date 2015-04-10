#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/timer.h>

static struct input_dev *input;
static struct timer_list timer;

static void timer_function(unsigned long data)
{
	printk(KERN_INFO "nunchuk timer!\n");

	input_report_rel(input, REL_X, 5);
	input_report_rel(input, REL_Y, -5);
	input_sync(input);

	mod_timer(&timer, jiffies + msecs_to_jiffies(100));
}

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

	input->name = "nunchuk";

	set_bit(EV_REL, input->evbit);
	set_bit(REL_X, input->relbit);
	set_bit(REL_Y, input->relbit);

	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_LEFT, input->keybit);
	set_bit(BTN_RIGHT, input->keybit);

	error = input_register_device(input);
	if (error) {
		printk(KERN_ERR "nunchuk_input_driver_init: "
			"failed to register input device (%d)\n", error);
		goto err_freedev;
	}

	setup_timer(&timer, timer_function, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(100));

	return 0;

err_freedev:
	input_free_device(input);
err_nomem:
	return error;
}

static void __exit nunchuk_input_driver_exit(void)
{
	printk(KERN_INFO "nunchuk_input_driver_exit()\n");

	del_timer_sync(&timer);
	input_unregister_device(input);
}

module_init(nunchuk_input_driver_init);
module_exit(nunchuk_input_driver_exit);

MODULE_AUTHOR("Sergi Granell");
MODULE_DESCRIPTION("nunchuk input driver");
MODULE_LICENSE("GPL");
