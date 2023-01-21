#include "../inc/bme280.h"
#include "../inc/i2c.h"

//Returns external temperature value. In case of any failure, retry.
float bme_start()
{
    struct bme280_dev dev;
    struct identifier id;

    int8_t rslt = BME280_OK;

    char i2cInterface[] = "/dev/i2c-1";
    if ((id.fd = open(i2cInterface, O_RDWR)) < 0)
    {
        return bme_start();
    }

    id.dev_addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0)
    {
        close(id.fd);
        return bme_start();
    }

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;
    dev.intf_ptr = &id;

    rslt = bme280_init(&dev);
    if (rslt != BME280_OK)
    {
        close(id.fd);
        return bme_start();
    }

    return stream_sensor_data_normal_mode(&dev, id);
}

float stream_sensor_data_normal_mode(struct bme280_dev *dev, struct identifier id)
{
    int8_t rslt;
    uint8_t settings_sel;
    struct bme280_data comp_data;

    //Recommended mode of operation: Indoor navigation
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;
    dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    settings_sel = BME280_OSR_PRESS_SEL;
    settings_sel |= BME280_OSR_TEMP_SEL;
    settings_sel |= BME280_OSR_HUM_SEL;
    settings_sel |= BME280_STANDBY_SEL;
    settings_sel |= BME280_FILTER_SEL;
    rslt = bme280_set_sensor_settings(settings_sel, dev);
    rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

    dev->delay_us(100000, dev->intf_ptr);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
    if (rslt != BME280_OK)
    {
        close(id.fd);
        return bme_start();
    }

    close(id.fd);
    return comp_data.temperature;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    usleep(period);
}

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}