#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>

#define GPIO_BASE 0xFF203000  // adresse physique de la GPIO des LED
#define MAP_SIZE  0x1000      // taille d’une page (4 KiB)

int main(void) {
    // 1. Ouvrir /dev/mem en lecture/écriture
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // 2. Remapper la page physique contenant nos registres
    void *map = mmap(NULL, MAP_SIZE,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     fd, GPIO_BASE);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // 3. Pointage vers le registre (offset 0)
    volatile uint32_t *reg = (uint32_t *)map;

    // 4. Écriture : exemple, allumer la LED 8 (bit 8)
    if(*reg >= 128)
    	*reg = 0;
    else
	*reg = (1 << 8);

    // 5. Nettoyage
    munmap(map, MAP_SIZE);
    close(fd);
    return EXIT_SUCCESS;
}

