#include <unistd.h>

#include <uart.h>

#define POTENTIOMETER_UPDATE_RATE 0.05 // 0.05s = 50 ms
#define POTENTIOMETER_SERIAL_BUS "/dev/serial0"

#define INTERNAL_SENSOR_UPDATE_RATE 0.05 // 0.05s = 50 ms
#define INTERNAL_SENSOR_SERIAL_BUS "/dev/serial1"

void* potentiometer_thread(void* potentiometer) {
    Uart uart = new_uart(POTENTIOMETER_SERIAL_BUS);

    while(1) {
        printf("Getting info from arduino\n");
        *(float *) potentiometer = get_float(&uart);
        sleep(POTENTIOMETER_UPDATE_RATE);
    }
}

void* internal_sensor_thread(void* internal_sensor) {
    /* Uart uart = new_uart(INTERNAL_SENSOR_SERIAL_BUS); */

    while(1) {
        /* printf("Getting info from arduino\n"); */
        /* *(float *) internal_sensor = get_float(&uart); */
        *(float *) internal_sensor += 1;
        sleep(INTERNAL_SENSOR_UPDATE_RATE);
    }
}
