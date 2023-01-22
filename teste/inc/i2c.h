#ifndef READ_BME280_
#define READ_BME280_

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "bme280.h"

struct identifier
{
    uint8_t dev_addr;
    int8_t fd;
};

void user_delay_us(uint32_t, void *);
int8_t user_i2c_read(uint8_t, uint8_t *, uint32_t, void *);
int8_t user_i2c_write(uint8_t, const uint8_t *, uint32_t, void *);
float stream_sensor_data_normal_mode(struct bme280_dev *, struct identifier);
float bme_start();

#endif