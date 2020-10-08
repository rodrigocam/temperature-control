#include <unistd.h>

#include <uart.h>

#define POTENTIOMETER_UPDATE_RATE 0.05 // 0.05s = 50 ms
#define POTENTIOMETER_SERIAL_BUS "/dev/serial0"

#define INTERNAL_SENSOR_UPDATE_RATE 0.05 // 0.05s = 50 ms
#define INTERNAL_SENSOR_SERIAL_BUS "/dev/serial1"

void* potentiometer_thread(void* potentiometer) {
    Uart uart = new_uart(POTENTIOMETER_SERIAL_BUS);

    while(1) {
        *(float *) potentiometer = get_float(&uart, CODE_GET_POTENTIOMETER);
        sleep(POTENTIOMETER_UPDATE_RATE);
    }
}

void* internal_sensor_thread(void* internal_sensor) {
    Uart uart = new_uart(INTERNAL_SENSOR_SERIAL_BUS);

    while(1) {
        *(float *) internal_sensor = get_float(&uart, CODE_GET_INTERNAL_TEMPERATURE);
        sleep(INTERNAL_SENSOR_UPDATE_RATE);
    }
}
