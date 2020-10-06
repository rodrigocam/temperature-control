#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CODE_GET_INT 0xA1
#define CODE_GET_FLOAT 0xA2
#define CODE_GET_STR 0xA3

#define PACKAGE_SIZE 5

const unsigned char AUTH_KEY[4] = {1, 3, 9, 9};

typedef struct Uart {
    char* serial_bus;
    struct termios options;
    int file_descriptor;
} Uart;

Uart new_uart(char* serial_bus) {
    Uart uart;

    uart.serial_bus = malloc(30 * sizeof(char));
    strcpy(uart.serial_bus, serial_bus);

    uart.file_descriptor = -1;

    uart.file_descriptor = open(serial_bus, O_RDWR | O_NOCTTY);

    if(uart.file_descriptor == -1) {
        fprintf(stderr, "Failed to open `%s`\n", serial_bus);
        exit(1);
    }

    tcgetattr(uart.file_descriptor, &uart.options);
    uart.options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //Set baud rate
    uart.options.c_iflag = IGNPAR;
    uart.options.c_oflag = 0;
    uart.options.c_lflag = 0;
    tcflush(uart.file_descriptor, TCIFLUSH);
    tcsetattr(uart.file_descriptor, TCSANOW, &uart.options);

    return uart;
}

void send_package(Uart* uart, unsigned char* package) {
    const int writed_size = write(uart->file_descriptor, &package, PACKAGE_SIZE); 
    
    if(writed_size < 0) {
        fprintf(stderr, "Failed to send package, error number %d:  `%s`\n", errno, strerror(errno));
        exit(1);
    }

    if(writed_size < sizeof(package)) {
        fprintf(stderr, "Failed to send package, impossible to write the entire package content\n");
        exit(1);
    }
}

void receive_package(Uart* uart, void* response, int package_size) {
    const int read_size = read(uart->file_descriptor, response, package_size);

    if(read_size < 0) {
        fprintf(stderr, "Failed to receive package, error number %d:  `%s`\n", errno, strerror(errno));
        exit(1);
    }

    if(read_size < package_size) {
        fprintf(stderr, "Failed to receive package, impossible to read entire response. Specified size: %d, Received size: %d\n", package_size, read_size);
        exit(1);
    }
}

float get_float(Uart* uart) {
    unsigned char response[4];
    unsigned char package[5] = {CODE_GET_FLOAT};
    strcat(package, AUTH_KEY);

    send_package(uart, package);
    sleep(0.05);
    receive_package(uart, response, sizeof(float));

    return *((float*) response);
}
