/**********************************************
 **** MPU6050 user-space device driver*********
 ******** By Shubham Santosh ******************
 *********************************************/

#include "mpu6050.h"

#define MPU6050_ADDR 0x68

int main()
{
	uint8_t buf[6];
	int16_t accel_x_raw, accel_y_raw, accel_z_raw;
	int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
	int16_t temp;
	int fd = i2c_open("/dev/i2c-1", MPU6050_ADDR);  // MPU6050 example
	if(fd<0)
	{
		printf("failed to connect to i2c device\n");
		return 0;
	}
	printf("I2C device connected\n");
	if(i2c_write(fd,0x6B,0x00)<0)
	{
		printf("I2C device wake up failed\n");
		return 0;
	}
	if(i2c_write(fd,0x1C,0x00)<0)
	{
		printf("I2C accelerometer config failed\n");
		return 0;
	}
	if(i2c_write(fd,0x1B,0x00)<0)
	{
		printf("I2C gyroscope config failed\n");
		return 0;
	}
	while(1)
	{
		
		if(i2c_read(fd,0x3B,buf,6)<0)
		{
			printf("Accelerometer reading failed\n");
			continue;
		}
		
		accel_x_raw = (int16_t)(buf[0]<<8 | buf[1]);
		accel_y_raw = (int16_t)(buf[2]<<8 | buf[3]);
		accel_z_raw = (int16_t)(buf[4]<<8 | buf[5]);
		
		if(i2c_read(fd,0x43,buf,6)<0)
		{
			printf("Gyro reading failed\n");
			continue;
		}
		gyro_x_raw = (int16_t)(buf[0]<<8 | buf[1]);
		gyro_y_raw = (int16_t)(buf[2]<<8 | buf[3]);
		gyro_z_raw = (int16_t)(buf[4]<<8 | buf[5]);
		
		if(i2c_read(fd,0x41,buf,2)<0)
		{
			printf("Temp reading failed\n");
			continue;
		}
		temp=(int16_t)(buf[0]<<8 | buf[1]);
		printf("-----------------------------------------------------------\n");
		printf("Temp reading: %.3f\n",temp/340.0f + 36.53f);
		//printf("X: %3f, Y: %3f, Z: %3f\n",x_axis, y_axis, z_axis);
		printf("Accelerometer X: %.3f Y: %.3f Z: %.3f\n",
           accel_x_raw / 16384.0f,
           accel_y_raw / 16384.0f,
           accel_z_raw / 16384.0f);
        printf("Gyro raw X: %d Y: %d Z: %d\n",
           accel_x_raw,
           accel_y_raw,
           accel_z_raw);
           
           usleep(1000000); //sleep for 1 second
	}
	return 0;
}
