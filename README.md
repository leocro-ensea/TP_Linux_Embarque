# TP_Linux_Embarque

### \### Commandes utiles :

arm-linux-gnueabihf-gcc simple_chenillard_arg.c -o simple_chenillard_arg  
file simple_chenillard_arg # doit indiquer "ELF 32-bit LSB, ARM"  
scp simple_chenillard_arg root@&lt;IP_SOC&gt;:/root [//192.168.0.204](//192.168.0.204)

### 1.3.2 Utilisez un logiciel de liaison série

Quelle est la taille occupée ? Notre carte SD fait 16G  
Avant le resize : /dev/root Size : 3.0G, Used : 1.3G  
Apres le resize : /dev/root Size : 14.0G, Used : 1.3G  
<br/>

### 1.4.3 Hello world !

Prog HelloWord :

```C
#include <stdio.h>

int main(void)
{
    printf("Hello World !\n");
    return 0;
}

```

### 1.4.5 Chenillard (Et oui, encore !)

```C
// simple_chenillard_arg.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define N_LEDS 8
#define DEFAULT_DELAY_MS 200  // délai par défaut en ms

int main(int argc, char *argv[]) {anche, voici pourquoi on l’utilise presque systématiquement :

Automatisation

Un seul make remplace plusieurs commandes longues et évite les fautes de frappe.

Si vous avez plusieurs fichiers source, make sait compiler seulement ce qui a changé.

Gestion des dépendances
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

```

### 2.1 Accès aux registres

```c
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

```

### 2.2 Compilation de module noyau sur la VM

#### Module Hello.c

```
#include <linux/init.h>      // pour les macros __init et __exit
#include <linux/module.h>    // pour MODULE_LICENSE, module_init, module_exit
#include <linux/kernel.h>    // pour printk

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Votre Nom");
MODULE_DESCRIPTION("Un module simple Hello World");
MODULE_VERSION("0.1");

static int __init hello_init(void)
{
    printk(KERN_INFO "hello: module chargé !\n");
    return 0; // 0 = succès
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "hello: module déchargé !\n");
}

module_init(hello_init);
module_exit(hello_exit);

```

#### Makefile

```Makefile
obj-m += hello.o

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

```

Comment compiler et insérer et retirer le module

```bash
make                   # compile hello.ko

sudo insmod hello.ko   # insère le module
sudo dmesg | tail -n 5 # affiche les 5 dernières lignes du log
# [...] hello: module chargé !

sudo rmmod hello       # retire le module
sudo dmesg | tail -n 5 # vérifiez le message de déchargement
# [...] hello: module déchargé !
```
