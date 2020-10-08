#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <bme280.h>
#include <i2c_bme.h>

#define SERIAL_BUS "/dev/i2c-1"
#define I2C_ADDR 0x76
#define UPDATE_RATE 0.5 // 500ms


typedef struct BME280 {
    struct identifier id;
    struct bme280_dev device;
}BME280;

BME280 new_bme280(char* serial_bus, uint8_t addr) {
    BME280 bme;

    struct identifier id;
    struct bme280_dev device;

    id.dev_addr = addr;

    int8_t result = BME280_OK;

    if((id.fd = open(serial_bus, O_RDWR)) < 0) {
        fprintf(stderr, "Failed to open i2c bus %s\n", serial_bus);
        exit(1);
    }
    
    if(ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        fprintf(stderr, "Failed to acquire bus access\n");
        exit(1);
    }

    device.intf = BME280_I2C_INTF;
    device.read = i2c_read;
    device.write = i2c_write;
    device.delay_us = delay_us;
    device.intf_ptr = &id;

    result = bme280_init(&device);
    
    if(result != BME280_OK) {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", result);
        exit(1);
    }

    bme.id = id;
    bme.device = device;
    return bme;
}

void stream_temperature(BME280 *sensor, float *temperature) {
    int8_t result = bme280_set_sensor_mode(BME280_NORMAL_MODE, &sensor->device);
    
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor mode (code %+d).", result);
        exit(1);
    }
    
    result = bme280_set_sensor_settings(BME280_OSR_TEMP_SEL, &sensor->device);
    
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", result);
        exit(1);
    }
    
    struct bme280_data sensor_data;
    uint32_t request_delay = bme280_cal_meas_delay(&sensor->device.settings);

    while(1) {
        sensor->device.delay_us(request_delay, sensor->device.intf_ptr);
        result = bme280_get_sensor_data(BME280_TEMP, &sensor_data, &sensor->device);

        if (result != BME280_OK) {
            fprintf(stderr, "Failed to get sensor data (code %+d).", result);
            exit(1);
        }
        
        *temperature = sensor_data.temperature;
        sleep(UPDATE_RATE);
    }
}

void* external_sensor_thread(void* external_temperature) {
    BME280 bme = new_bme280(SERIAL_BUS, I2C_ADDR);
    stream_temperature(&bme, (float *) external_temperature);
}
