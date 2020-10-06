#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>
#include <curses.h>

#include <arduino.h>
#include <external_sensor.h>

#define POTENTIOMETER_ON "On"
#define POTENTIOMETER_OFF "Off"

static float POTENTIOMETER_VALUE = 0;
static float INTERNAL_SENSOR_VALUE = 0;
static float EXTERNAL_SENSOR_VALUE = 0;

void* input_thread(void* option) {
    WINDOW *menu_window = newwin(10, 40, 35, 45);
    box(menu_window, 0, 0);
    while(1)
        mvwprintw((WINDOW*) menu_window, 2, 5, "P - To switch potenciometer state");
        mvwscanw((WINDOW*) menu_window, 6, 5, "%d", &option);
        wrefresh((WINDOW*) menu_window);
}

int main(void) {

    int row, col;
    int option = 0;

    initscr();
    noecho();
    nodelay(stdscr, TRUE);
    cbreak();
    getmaxyx(stdscr, row, col);

    WINDOW *temperature_window = newwin(10, 40, 5, 5);
    box(temperature_window, 0, 0);

    WINDOW *status_window = newwin(10, 40, 5, 45);
    box(status_window, 0, 0);
    
    WINDOW *menu_window = newwin(10, 40, 35, 45);
    box(menu_window, 0, 0);

    curs_set(0);
    
    pthread_t potentiometer_tid;
    pthread_t internal_sensor_tid;
    pthread_t external_sensor_tid;
    /* pthread_t input_tid; */

    /* pthread_create(&potentiometer_tid, NULL, potentiometer_thread, (void *)&POTENTIOMETER_VALUE); */
    pthread_create(&internal_sensor_tid, NULL, internal_sensor_thread, (void *)&INTERNAL_SENSOR_VALUE);
    pthread_create(&external_sensor_tid, NULL, external_sensor_thread, (void *)&EXTERNAL_SENSOR_VALUE);

    /* pthread_create(&input_tid, NULL, input_thread, (void *)&option); */
    
    while(1) {
        /* mvprintw(row/2, col/2, "Internal Temperature: %.2f", INTERNAL_SENSOR_VALUE); */
        mvwprintw(temperature_window, 2, 5, "Internal Temperature: %0.2f", INTERNAL_SENSOR_VALUE);
        mvwprintw(temperature_window, 6, 5, "External Temperature: %0.2f", EXTERNAL_SENSOR_VALUE);
   
        if(option == 1) {
            mvwprintw(status_window, 2, 5, "Potentiometer: %s", POTENTIOMETER_ON);
            mvwprintw(status_window, 6, 5, "Potentiometer Value: %s", POTENTIOMETER_VALUE);
        } else {
            mvwprintw(status_window, 2, 5, "Potentiometer: %s", POTENTIOMETER_OFF);
        }

        mvwprintw(menu_window, 2, 5, "P - To switch potenciometer state");
        char ch = (int) getch();
        option = (int) ch; 
        /* mvwscanw(menu_window, 6, 5, "%d", &option); */
        
        wrefresh(temperature_window);
        wrefresh(status_window);
        wrefresh(menu_window);
        /* mvscanw(row/2 + 1, col/2, "%d", &option); */
        /* printw("Internal Temperature: %.2f", INTERNAL_SENSOR_VALUE); */
        /* refresh(); */
    }
}
