# ─────────────────────────────────────────────────────────────
# Makefile pour compiler creation_entree_proc.c en module ARM
# Noyau 4.5.0-00198-G6B20A29 (linux-socfpga)
# ─────────────────────────────────────────────────────────────
obj-m := creation_entree_proc.o

# Chemin vers l’arbre complet du noyau ARM préparé (étape précédente)
KDIR := /home/ensea/linux-socfpga

# Toolchain croisée ARM
ARCH          := arm
CROSS_COMPILE := arm-linux-gnueabihf-

# Répertoire courant du module
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

