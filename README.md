# Compte rendu de TP de Linux Embarqué
#### Commandes utiles :

arm-linux-gnueabihf-gcc simple_chenillard_arg.c -o simple_chenillard_arg  
file simple_chenillard_arg # doit indiquer "ELF 32-bit LSB, ARM"  
scp simple_chenillard_arg root@&lt;IP_SOC&gt;:/root [//192.168.0.204](//192.168.0.204)

## 1.3 Connexion au système
### 1.3.2 Utilisez un logiciel de liaison série

Quelle est la taille occupée ? Notre carte SD fait 16G 
Avant le resize : /dev/root Size : 3.0G, Used : 1.3G
Apres le resize : /dev/root Size : 14.0G, Used : 1.3G

### 1.4.3 Hello world !
Dans cette partie, nous écrivons des programmes C simples exécutés sur la carte ARM depuis l’espace utilisateur.

Prog HelloWord :
```C
#include <stdio.h>

int main(void)
{
    printf("Hello World !\n");
    return 0;
}

```

Compilation :
```bash
arm-linux-gnueabihf-gcc hello.c -o hello.o
file hello.o   # ELF 32-bit ARM
```

Exécution sur la carte : ./hello.o ➡︎ Hello World !

### 1.4.5 Chenillard (Et oui, encore !)
 Accès LED via /sys
Écriture directe : echo 1 > /sys/class/leds/fpga_led1/brightness
Lecture état : cat /sys/class/leds/fpga_led1/brightness

Programme simple_chenillard_arg.c : tempo passé en argument
```C
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
```
arm-linux-gnueabihf-gcc simple_chenillard_arg.c -o simple_chenillard_arg
scp simple_chenillard_arg root@SOC:/root && ./simple_chenillard_arg 200

### 2.1 Accès aux registres
Programme user-space qui mappe 0xFF203000 (GPIO) et allume la LED 8.

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
make                   # compile hello.ko (module noyau)

modinfo hello.ko       # infos sur le module (.ko) : auteur, licence, vermagic
lsmod                  # liste les modules chargés
sudo insmod hello.ko   # insère le module dans le noyau

dmesg | tail -n 5      # affiche les 5 derniers messages du noyau
# [...] hello: module chargé !

sudo rmmod hello       # retire le module du noyau
dmesg | tail -n 5      # vérifie le message de déchargement
# [...] hello: module déchargé !
```

Création d'une entree proc :

```C
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Léonard");
MODULE_DESCRIPTION("Creation entree proc");
MODULE_SUPPORTED_DEVICE("Tous");
MODULE_LICENSE("GPL");

#define BUF_LEN 64

static ssize_t my_read_function(struct file *file, char *buf, size_t count, loff_t *ppos) {
    int lus = 0;

    printk(KERN_DEBUG "read: demande lecture de %d octets\n", count);
    
    /* Check for overflow */
    if (count <= BUF_LEN - (int)*ppos)
        lus = count;
    else lus = BUF_LEN - (int)*ppos;

    if(lus)
        copy_to_user(buf, (int *)buf + (int)*ppos, lus);
    
    *ppos += lus;

    printk(KERN_DEBUG "read: %d octets reellement lus\n", lus);
    printk(KERN_DEBUG "read: position=%d\n", (size_t)*ppos);

    return lus;
}

static ssize_t my_write_function(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    int ecrits = 0, i = 0;

    printk(KERN_DEBUG "write: demande ecriture de %d octets\n", count);

    if (count <= BUF_LEN - (int)*ppos) /* Check for overflow */
        ecrits = count;
    else ecrits = BUF_LEN - (int)*ppos;

    if(ecrits)
        copy_from_user((int *)buf + (int)*ppos, buf, ecrits);
    *ppos += ecrits;

    printk(KERN_DEBUG "write: %d octets reellement ecrits\n", ecrits);
    printk(KERN_DEBUG "write: position=%d\n", (int)*ppos);
    printk(KERN_DEBUG "write: contenu du buffer\n");

    for(i=0;i<BUF_LEN;i++)
        printk(KERN_DEBUG " %d", buf[i]);

    printk(KERN_DEBUG "\n");
    return ecrits;
}

static int my_open_function(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "open()\n");
    return 0;
}

static int my_release_function(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "close()\n");
    return 0;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read_function,
    .write = my_write_function,
    .open = my_open_function,
    .release = my_release_function /* correspond a close */
};

static int __init simple_module_init(void) {
    if (proc_create("test_proc", 0666, NULL, &fops) >= 0)
    {
        printk(KERN_INFO, "Fichier créé\n");
    }
    else
    {
        printk(KERN_INFO, "Echec création fichier\n");
        return -1;
    }
    return 0;
}

static void __exit simple_module_exit(void) {
    remove_proc_entry("test_proc", NULL);
    printk(KERN_INFO, "Fichier supprimé\n");
    printk(KERN_INFO, "Module déchargé\n");
}

module_init(simple_module_init);
module_exit(simple_module_exit);
```

Durant ce TP, nous avons rencontré plusieurs difficultés à comprendre clairement la différence entre compiler un module pour la machine virtuelle et pour la carte cible ARM. Nous avons souvent mélangé les fichiers et les Makefiles, ce qui a provoqué des erreurs répétées au moment d'insérer les modules sur la carte (par exemple « Invalid module format »). Nous avons passé énormément de temps à essayer de déboguer ces problèmes sans toujours réussir à avancer autant que nous l'aurions voulu. Malgré ces difficultés, nous avons compris progressivement les erreurs commises, mais nous n’avons malheureusement pas pu terminer intégralement le TP.
