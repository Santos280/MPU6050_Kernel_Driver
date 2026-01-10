#include "mpu6050.h"

int i2c_open(const char* i2c_dev_name, uint8_t address)
{
	int fd=open(i2c_dev_name,O_RDWR);
	if(fd < 0)
	{
		perror("Failed to open I2C driver\n");
		return -1;
	}
	if(ioctl(fd,I2C_SLAVE, address)<0)
	{
		perror("Failed to set I2C address\n");
		close(fd);
		return -1;
	}
	return fd;
}

int i2c_read(int fd, uint8_t reg, uint8_t * val, uint8_t bytes_to_read)
{
	if (write(fd, &reg, 1) != 1) {
        perror("I2C register select failed");
        return -1;
    }

    if (read(fd, val, bytes_to_read) != bytes_to_read) {
        perror("I2C read failed");
        return -1;
    }
    return 0;
}

int i2c_write(int fd, uint8_t reg, uint8_t val)
{
	 uint8_t buf[2] = { reg, val };

    if (write(fd, buf, 2) != 2) {
        perror("I2C write failed");
        return -1;
    }
    return 0;
}
