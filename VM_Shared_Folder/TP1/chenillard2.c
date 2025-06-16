// simple_chenillard_arg.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define N_LEDS 8
#define DEFAULT_DELAY_MS 200  // délai par défaut en ms

int main(int argc, char *argv[]) {
    char path[64];
    int fd;
    int delay_ms = DEFAULT_DELAY_MS;

    // Lecture de l'argument tempo (en ms)
    if (argc >= 2) {
        int arg = atoi(argv[1]);
        if (arg > 0) {
            delay_ms = arg;
        } else {
            fprintf(stderr, "Usage : %s [delay_ms > 0]\n", argv[0]);
            return 1;
        }
    }

    // Convertir en microsecondes pour usleep
    int delay_us = delay_ms * 1000;

    while (1) {
        for (int i = 1; i <= N_LEDS; i++) {
            // Allumer
            snprintf(path, sizeof(path),
                     "/sys/class/leds/fpga_led%d/brightness", i);
            fd = open(path, O_WRONLY);
            if (fd >= 0) {
                write(fd, "1", 1);
                close(fd);
            }

            usleep(delay_us);

            // Éteindre
            fd = open(path, O_WRONLY);
            if (fd >= 0) {
                write(fd, "0", 1);
                close(fd);
            }
        }
    }

    return 0;
}

