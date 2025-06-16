#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define N_LEDS 9
#define DEFAULT_DELAY_MS 200

static int fds[N_LEDS];
static volatile int keep_running = 1;

void sigint_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    int delay = DEFAULT_DELAY_MS;
    if (argc >= 2) {
        delay = atoi(argv[1]);
        if (delay <= 0) delay = DEFAULT_DELAY_MS;
    }

    /* Installer le handler pour Ctrl+C */
    signal(SIGINT, sigint_handler);

    /* Ouvrir tous les fichiers brightness en écriture */
    char path[64];
    for (int i = 0; i < N_LEDS; i++) {
        snprintf(path, sizeof(path),
                 "/sys/class/leds/fpga_led%d/brightness", i+1);
        fds[i] = open(path, O_WRONLY);
        if (fds[i] < 0) {
            perror(path);
            return 1;
        }
    }

    /* Boucle de balayage */
    int idx = 0, dir = 1;
    while (keep_running) {
        /* Éteindre toutes les LED */
        for (int i = 0; i < N_LEDS; i++)
            write(fds[i], "0", 1);

        /* Allumer la LED courante */
        write(fds[idx], "1", 1);

        usleep(delay * 1000);

        /* Avancer ou reculer dans le pattern */
        idx += dir;
        if (idx == N_LEDS-1 || idx == 0)
            dir = -dir;
    }

    /* Avant de quitter, éteindre toutes les LED */
    for (int i = 0; i < N_LEDS; i++) {
        write(fds[i], "0", 1);
        close(fds[i]);
    }

    return 0;
}
