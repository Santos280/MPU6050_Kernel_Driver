#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

int i2c_open(const char* i2c_dev_name, uint8_t address);

int i2c_read(int fd, uint8_t reg, uint8_t * result,uint8_t bytes_to_read);
//int i2c_read_multi(int fd, uint8_t reg, uint8_t * val, uint8_t bytes_to_read);

int i2c_write(int fd, uint8_t reg, uint8_t val);
