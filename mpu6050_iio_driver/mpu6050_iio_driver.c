/**************************************************************
 ***** IIO Linux Kernel driver with buffer streaming **********
 **************** By Shubham Santosh **************************
 *************************************************************/
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>


#define MPU6050_ACCEL_XOUT_H   0x3B
#define MPU6050_GYRO_XOUT_H    0x43
#define MPU6050_TEMP_OUT_H     0x41

#define MPU6050_ACCEL_CHAN(_axis, _idx) {        \
    .type = IIO_ACCEL,                           \
    .modified = 1,                               \
    .channel2 = _axis,                           \
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),\
    .scan_index = _idx,                          \
    .scan_type = {                               \
        .sign = 's',                             \
        .realbits = 16,                          \
        .storagebits = 16,                       \
        .shift = 0,                              \
        .endianness = IIO_BE,                    \
    },                                           \
}

struct mpu6050_data {
    struct i2c_client *client;
    struct mutex lock;
};

static int mpu6050_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val,int *val2,long mask);
static irqreturn_t mpu6050_trigger_handler(int irq, void *p);

static const struct iio_info mpu6050_info = {
    .read_raw = mpu6050_read_raw,
};

static const struct iio_chan_spec mpu6050_channels[] = {
    /* Accelerometer X */
    {
        .type = IIO_ACCEL,
        .modified = 1,
        .channel2 = IIO_MOD_X,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .scan_index = 0,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .shift = 0,
            .endianness = IIO_BE,
        },
    },
    MPU6050_ACCEL_CHAN(IIO_MOD_Y, 1),
    MPU6050_ACCEL_CHAN(IIO_MOD_Z, 2),
    
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

static int mpu6050_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val,int *val2,long mask)
{
    struct mpu6050_data *data = iio_priv(indio_dev);
    int ret=0;

    mutex_lock(&data->lock);

    switch (mask) {
    case IIO_CHAN_INFO_RAW:
        /* determine register using chan->type and chan->channel2 */
	switch(chan->channel2)
	{
	    case IIO_MOD_X:
		ret = mpu6050_read_word(data->client, MPU6050_ACCEL_XOUT_H);
		break;
	    case IIO_MOD_Y:
		ret = mpu6050_read_word(data->client, MPU6050_ACCEL_XOUT_H+2);
		break;
	    case IIO_MOD_Z:
		ret = mpu6050_read_word(data->client, MPU6050_ACCEL_XOUT_H+4);
		break;
	    default:
		ret=-EINVAL;
	}
	break;
	default:
	    ret=-EINVAL;
    }
    mutex_unlock(&data->lock);
    //if (ret < 0) { return ret;
	//	}
	    *val = ret;
	    return IIO_VAL_INT;
}

static irqreturn_t mpu6050_trigger_handler(int irq, void *p)
{
    struct iio_poll_func *pf = p;
    struct iio_dev *indio_dev = pf->indio_dev;
    struct mpu6050_data *data = iio_priv(indio_dev);
    s16 accel[3];

    mutex_lock(&data->lock);

    accel[0] = mpu6050_read_word(data->client, MPU6050_ACCEL_XOUT_H);
    accel[1] = mpu6050_read_word(data->client, MPU6050_ACCEL_XOUT_H + 2);
    accel[2] = mpu6050_read_word(data->client, MPU6050_ACCEL_XOUT_H + 4);

    mutex_unlock(&data->lock);

    iio_push_to_buffers(indio_dev, accel);
    iio_trigger_notify_done(indio_dev->trig);

    return IRQ_HANDLED;
}


static int my_probe(struct i2c_client *client)
{
	int whoami = i2c_smbus_read_byte_data(client, 0x75);
	//printk(KERN_ERR "WHO AM I: %d\n",whoami);
	dev_info(&client->dev, "WHO_AM_I = 0x%x\n", whoami);
	struct mpu6050_data *data;
	struct iio_dev* indio_dev;
	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
	    return -ENOMEM;
	
	data = iio_priv(indio_dev);
	data->client = client;
	mutex_init(&data->lock);
	
	/* Basic IIO device setup */
	indio_dev->dev.parent = &client->dev;
	indio_dev->info       = &mpu6050_info;      /* read_raw lives here */
	indio_dev->modes      = INDIO_DIRECT_MODE;//| INDIO_BUFFER_SOFTWARE;
	indio_dev->channels   = mpu6050_channels;   /* accel + gyro axes */
	indio_dev->num_channels = ARRAY_SIZE(mpu6050_channels);
	indio_dev->name       = "mpu6050_iio";
	
	
	    /* Wake up and configure MPU6050 */
	int ret = i2c_smbus_write_byte_data(client, 0x6B, 0x00); /* PWR_MGMT_1 */
	if (ret < 0)
	    return ret;

	ret = i2c_smbus_write_byte_data(client, 0x1C, 0x00); /* Accel ±2g */
	if (ret < 0)
	    return ret;

	ret = i2c_smbus_write_byte_data(client, 0x1B, 0x00); /* Gyro ±250 dps */
	if (ret < 0)
	    return ret;
	    
	ret = devm_iio_triggered_buffer_setup(&client->dev,indio_dev,
	NULL,                        /* preenable */
        mpu6050_trigger_handler,     /* pollfunc */
        NULL); 
	/* Register with IIO core (creates sysfs automatically) */
	ret = iio_device_register(indio_dev);
	if (ret)
	    return ret;
	                      /* postdisable */

	/* Store pointer for remove() */
	i2c_set_clientdata(client, indio_dev);

	dev_info(&client->dev, "MPU6050 IIO device registered\n");
	    return 0;
}

static void my_remove(struct i2c_client *client)
{
    struct iio_dev *indio_dev = i2c_get_clientdata(client);
    iio_device_unregister(indio_dev);
    //iio_triggered_buffer_cleanup(indio_dev);
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
MODULE_AUTHOR("Shubham");
MODULE_DESCRIPTION("IIO driver for MPU6050"); 

