#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CODE_GET_INTERNAL_TEMPERATURE 0xA1
#define CODE_GET_POTENTIOMETER 0xA2

#define PACKAGE_SIZE 5

const unsigned char AUTH_KEY[4] = {1, 3, 9, 9};

typedef struct Uart {
    int file_descriptor;
} Uart;

Uart new_uart(char* serial_bus) {
    Uart uart;
    struct termios options;

    uart.file_descriptor = -1;

    uart.file_descriptor = open(serial_bus, O_RDWR | O_NOCTTY);

    if(uart.file_descriptor == -1) {
        fprintf(stderr, "Failed to open `%s`\n", serial_bus);
        exit(1);
    }

    tcgetattr(uart.file_descriptor, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart.file_descriptor, TCIFLUSH);
    tcsetattr(uart.file_descriptor, TCSANOW, &options);

    return uart;
}

void send_package(Uart* uart, unsigned char* package, size_t package_size) {
    const int writed_size = write(uart->file_descriptor, package, package_size); 
    
    if(writed_size < 0) {
        fprintf(stderr, "Failed to send package, error number %d:  `%s`\n", errno, strerror(errno));
        exit(1);
    }

    if(writed_size < package_size) {
        fprintf(stderr, "Failed to send package, impossible to write the entire package content\n");
        exit(1);
    }
}

void receive_package(Uart* uart, void* response, size_t package_size) {
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

float get_float(Uart* uart, char command) {
    unsigned char response[4];
    unsigned char package[5] = {command, 1, 3, 9, 9};

    send_package(uart, package, sizeof(package));
    sleep(1);
    receive_package(uart, response, sizeof(float));

    return *((float*) response);
}
