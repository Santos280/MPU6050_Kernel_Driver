/*********************************************
 ****** MPU6050 sysfs Kernel Driver **********
 ******** By Shubham Santosh *****************
 ********************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/mutex.h>


#define MPU6050_ACCEL_XOUT_H   0x3B
#define MPU6050_GYRO_XOUT_H    0x43
#define MPU6050_TEMP_OUT_H     0x41


struct mpu6050_data {
    struct i2c_client *client;
    struct mutex lock;
};


static int mpu6050_read_word(struct i2c_client *client, u8 reg)
{
    int msb, lsb;

    msb = i2c_smbus_read_byte_data(client, reg);
    if (msb < 0)
        return msb;

    lsb = i2c_smbus_read_byte_data(client, reg + 1);
    if (lsb < 0)
        return lsb;

    return (s16)((msb << 8) | lsb);
}

static ssize_t accel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct mpu6050_data *data = i2c_get_clientdata(client);
    //int val;

    mutex_lock(&data->lock);
    int16_t accel_x = mpu6050_read_word(client, MPU6050_ACCEL_XOUT_H);
    int16_t accel_y = mpu6050_read_word(client, MPU6050_ACCEL_XOUT_H+2);
    int16_t accel_z = mpu6050_read_word(client, MPU6050_ACCEL_XOUT_H+4);
    mutex_unlock(&data->lock);

   // if (accel_x  < 0 || accel_y <0|| accel_z<0)
    //    return accel_x;

    return sprintf(buf, "X: %d, Y: %d, Z: %d\n", accel_x,accel_y,accel_z);
}

static DEVICE_ATTR_RO(accel);

static ssize_t gyro_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct mpu6050_data *data = i2c_get_clientdata(client);
    //int val;

    mutex_lock(&data->lock);
    int16_t gyro_x = mpu6050_read_word(client, MPU6050_GYRO_XOUT_H);
    int16_t gyro_y = mpu6050_read_word(client, MPU6050_GYRO_XOUT_H+2);
    int16_t gyro_z = mpu6050_read_word(client, MPU6050_GYRO_XOUT_H+4);
    mutex_unlock(&data->lock);

    return sprintf(buf, "X: %d, Y: %d, Z: %d\n", gyro_x, gyro_y, gyro_z);
}


static DEVICE_ATTR_RO(gyro);


static int my_probe(struct i2c_client *client)
{
	int val = i2c_smbus_read_byte_data(client, 0x75);
	//dev_info(&client->dev, "I2C device probed\n");
	dev_info(&client->dev, "WHO_AM_I = 0x%x\n", val);
	
	val = i2c_smbus_write_byte_data(client, 0x1C, 0x00);  //wake-up
	if(val==0)
	dev_info(&client->dev, "I2C wakeup Initialized\n");
	else
	dev_err(&client->dev, "I2C wakeup Failed\n");
	
	val = i2c_smbus_write_byte_data(client, 0x6B, 0x00); //accel config
	if(val==0)
	dev_info(&client->dev, "I2C Accelerometer configured\n");
	else
	dev_err(&client->dev, "I2C Accelerometer configuration failed\n");
	
	val = i2c_smbus_write_byte_data(client, 0x1B, 0x00); // gyro config
	if(val==0)
	dev_info(&client->dev, "I2C Gyro configured\n");
	else
	dev_err(&client->dev, "I2C Gyro configuration failed\n");
	
	struct mpu6050_data *data;

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
	    return -ENOMEM;

	data->client = client;
	mutex_init(&data->lock);

	/* store pointer for later use in sysfs callbacks */
	i2c_set_clientdata(client, data);

	/* create sysfs attributes */
	device_create_file(&client->dev, &dev_attr_accel);
	//device_create_file(&client->dev, &dev_attr_config_accel);
	device_create_file(&client->dev, &dev_attr_gyro);

	return 0;
}

static void my_remove(struct i2c_client *client)
{
    device_remove_file(&client->dev, &dev_attr_accel);
    //device_remove_file(&client->dev, &dev_attr_config_accel);
    device_remove_file(&client->dev, &dev_attr_gyro);

    dev_info(&client->dev, "I2C device removed\n");
}

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


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shubham Santosh Kumar");
MODULE_DESCRIPTION("Sysfs device driver for MPU6050"); 

