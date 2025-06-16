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

