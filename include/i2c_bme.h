#include <bme280.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define CSV_FORMAT "%f, %f, %f, %02d/%02d/%02d:%02d:%02d:%02d\n"

struct identifier {
    uint8_t dev_addr;
    int8_t fd;
};

int8_t i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    struct identifier id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

int8_t i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    
    if (write(id.fd, buf, len + 1) < (uint16_t)len) {
        return BME280_E_COMM_FAIL;
    }
    
    free(buf);

    return BME280_OK;
}

void delay_us(uint32_t period, void *intf_ptr) {
    usleep(period);
}

void print_sensor_data(struct bme280_data *complete_data) {
    float temp, press, hum;
    
    temp = complete_data->temperature;
    press = 0.01 * complete_data->pressure;
    hum = complete_data->humidity;
    printf("%0.2lf deg C, %0.2lf hPa, %0.2lf%%\n", temp, press, hum);
}

void write_csv(float temperature, float pressure, float humidity) {
    time_t t = time(NULL);
    struct tm ts = *localtime(&t);

    FILE *f = fopen("measures.csv", "a");

    fprintf(f, CSV_FORMAT, temperature, pressure, humidity, ts.tm_mday, ts.tm_mon + 1, ts.tm_year + 1900, ts.tm_hour, ts.tm_min, ts.tm_sec);
    
    fclose(f);
}

int8_t stream_sensor_data_forced_mode(struct bme280_dev *device) {
    int8_t result = BME280_OK;

    uint8_t settings_selection = 0;

    uint32_t request_delay;

    struct bme280_data complete_data;

    device->settings.osr_h = BME280_OVERSAMPLING_1X;
    device->settings.osr_p = BME280_OVERSAMPLING_16X;
    device->settings.osr_t = BME280_OVERSAMPLING_2X;
    device->settings.filter = BME280_FILTER_COEFF_16;

    settings_selection = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    result = bme280_set_sensor_settings(settings_selection, device);
    
    if (result != BME280_OK)
    {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", result);

        return result;
    }

    request_delay = bme280_cal_meas_delay(&device->settings);

    int measure_count = 0;
    double temperature_acc, pressure_acc, humidity_acc;
    temperature_acc = pressure_acc = humidity_acc = 0;

    /* Continuously stream sensor data */
    while (1)
    {
        measure_count +=1;
        
        /* Set the sensor to forced mode */
        result = bme280_set_sensor_mode(BME280_FORCED_MODE, device);
        if (result != BME280_OK)
        {
            fprintf(stderr, "Failed to set sensor mode (code %+d).", result);
            break;
        }

        /* Wait for the measurement to complete */
        device->delay_us(request_delay, device->intf_ptr);
        result = bme280_get_sensor_data(BME280_ALL, &complete_data, device);

        if (result != BME280_OK)
        {
            fprintf(stderr, "Failed to get sensor data (code %+d).", result);
            break;
        }
       
        print_sensor_data(&complete_data);

        temperature_acc += complete_data.temperature;
        pressure_acc += complete_data.pressure;
        humidity_acc += complete_data.humidity;
        
        /* Write to CSV when 10 measures were captured */
        if(measure_count == 10) {
            printf("****** Registering new measures! ******\n");
            write_csv(temperature_acc / 10, pressure_acc / 10, humidity_acc / 10);
            temperature_acc = pressure_acc = humidity_acc = measure_count = 0;
        }
    }

    return result;
}

