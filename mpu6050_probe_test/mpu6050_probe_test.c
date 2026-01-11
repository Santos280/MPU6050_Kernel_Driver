/*****************************************
 ***** Skeleton code to check probe*******
 *********By Shubham Santosh *************
 ****************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/delay.h>

static int my_probe(struct i2c_client *client)
{
	int whoami = i2c_smbus_read_byte_data(client, 0x75);
	//dev_info(&client->dev, "I2C device probed\n");
	dev_info(&client->dev, "WHO_AM_I = 0x%x\n", whoami);

	return 0;
}

static void my_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "I2C device removed\n");
}
/*
struct i2c_client {
    unsigned short addr;      // 7-bit I2C address
    struct i2c_adapter *adapter;
    struct device dev;
};*/
static const struct of_device_id my_of_match[] = {
    { .compatible = "invensense,mpu6050_test" },
    { }
};
MODULE_DEVICE_TABLE(of, my_of_match);

static struct i2c_driver my_i2c_driver = {
    .driver = {
        .name = "mpu6050_test",
        .of_match_table = my_of_match,
    },
    .probe    = my_probe,
    .remove   = my_remove,
};


module_i2c_driver(my_i2c_driver);

//module_init(my_init);
//module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shubham");
MODULE_DESCRIPTION("A sample driver for register device"); 

