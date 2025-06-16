#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Claire");
MODULE_DESCRIPTION("Un module simple pour écrire dans le journal du noyau");
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
