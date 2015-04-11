#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/input.h>
#include <linux/i2c.h>

struct nunchuk_device {
	struct i2c_client 	*client;
	struct input_dev	*input;
	struct workqueue_struct	*wq;
	struct delayed_work	dwork;
	int			refn;
};

static atomic_t refcount = ATOMIC_INIT(0);

static inline struct nunchuk_device *to_nunchuk_device(struct work_struct *work)
{
	return container_of(to_delayed_work(work), struct nunchuk_device, dwork);
}

static void workqueue_function(struct work_struct *work)
{
	struct nunchuk_device *nunchuk;
	char x;

	nunchuk = to_nunchuk_device(work);
	
	printk(KERN_INFO "nunchuk %d timer!\n", nunchuk->refn);

	//i2c_master_recv(nunchuk->client, &x, sizeof(x));

	x = i2c_smbus_read_byte_data(nunchuk->client, 0x0);

	printk(KERN_INFO "nunchuk(%d) X = %d\n", nunchuk->refn, x);
	
	/*input_report_rel(input, REL_X, 5);
	input_report_rel(input, REL_Y, -5);
	input_sync(input);*/
	queue_delayed_work(nunchuk->wq, &nunchuk->dwork, msecs_to_jiffies(100));
}

static void init_nunchuk(struct nunchuk_device *nunchuk)
{
	static const char buf1[] = {0xF0, 0x55};
	static const char buf2[] = {0xFB, 0x00};

	//i2c_master_send(nunchuk->client, buf1, sizeof(buf1));
	//i2c_master_send(nunchuk->client, buf2, sizeof(buf2));

	i2c_smbus_write_block_data(nunchuk->client, 0, sizeof(buf1), buf1);
	i2c_smbus_write_block_data(nunchuk->client, 0, sizeof(buf2), buf2);
}

static int nunchuk_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct nunchuk_device *nunchuk;
	int err;

	printk(KERN_INFO "nunchuk_i2c_probe()\n");

	nunchuk = kzalloc(sizeof(*nunchuk), GFP_KERNEL);
	if (nunchuk == NULL) {
		return -ENOMEM;
	}

	nunchuk->client = client;
	nunchuk->input = input_allocate_device();
	if (nunchuk->input == NULL) {
		printk(KERN_ERR "nunchuk_i2c_probe: "
			"not enough memory\n");
		err = -ENODEV;
		goto err_freenun;
	}

	nunchuk->refn = atomic_inc_return(&refcount) - 1;
	nunchuk->input->name = "nunchuk";
	i2c_set_clientdata(client, nunchuk);

	set_bit(EV_REL, nunchuk->input->evbit);
	set_bit(REL_X, nunchuk->input->relbit);
	set_bit(REL_Y, nunchuk->input->relbit);

	set_bit(EV_KEY, nunchuk->input->evbit);
	set_bit(BTN_LEFT, nunchuk->input->keybit);
	set_bit(BTN_RIGHT, nunchuk->input->keybit);

	err = input_register_device(nunchuk->input);
	if (err) {
		printk(KERN_ERR "nunchuk_i2c_probe: "
			"failed to register input device (%d)\n", err);
		goto err_freedev;
	}

	init_nunchuk(nunchuk);

	nunchuk->wq = create_singlethread_workqueue("nunchuk");
	INIT_DELAYED_WORK(&nunchuk->dwork, workqueue_function);

	queue_delayed_work(nunchuk->wq, &nunchuk->dwork, msecs_to_jiffies(100));

	return 0;

err_freedev:
	input_free_device(nunchuk->input);
err_freenun:
	kfree(nunchuk);
	return err;
}

static int nunchuk_i2c_remove(struct i2c_client *client)
{
	struct nunchuk_device *nunchuk;

	printk(KERN_INFO "nunchuk_i2c_remove()\n");

	nunchuk = i2c_get_clientdata(client);

	cancel_delayed_work(&nunchuk->dwork);
	flush_workqueue(nunchuk->wq);
	destroy_workqueue(nunchuk->wq);

	input_unregister_device(nunchuk->input);

	kfree(nunchuk);
	
	return 0;
}


static const struct i2c_device_id nunchuk_i2c_idtable[] = {
	{"nunchuk-i2c", 0},
	{ }
};

MODULE_DEVICE_TABLE(i2c, nunchuk_i2c_idtable);

static struct i2c_driver nunchuk_i2c_driver = {
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "nunchuk-i2c",
	},
	.id_table	= nunchuk_i2c_idtable,
	.probe		= nunchuk_i2c_probe,
	.remove		= nunchuk_i2c_remove,
};



static int __init nunchuk_i2c_init(void)
{
	int err;

	printk(KERN_INFO "nunchuk_i2c_init()\n");
	
	err = i2c_add_driver(&nunchuk_i2c_driver);
	if (err) {
		printk(KERN_ERR "nunchuk_i2c_init: "
			"failed to add I2C driver (%d)\n", err);
		return err;
	}

	return 0;
}

static void __exit nunchuk_i2c_exit(void)
{
	printk(KERN_INFO "nunchuk_i2c_exit()\n");

	i2c_del_driver(&nunchuk_i2c_driver);
}

module_init(nunchuk_i2c_init);
module_exit(nunchuk_i2c_exit);

MODULE_AUTHOR("Sergi Granell");
MODULE_DESCRIPTION("Nunchuk I2C driver");
MODULE_LICENSE("GPL");
