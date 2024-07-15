#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");

int dev_major = 0;
int dev_minor = 1;
unsigned int n_drivers = 1;

dev_t dev;
struct cdev chard_cdev;
struct class * clase;
struct device * dispositivo;

static int chard_open(struct inode *inode, struct file *filp) {
    printk("Abierto\n");
    return 0;
}

static int chard_release(struct inode *inode, struct file *filp) {
    printk("Cerrado\n");
    return 0;
}

static ssize_t chard_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    printk("Escrito\n");
    return count;
}

static ssize_t chard_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    printk("Leido\n");
    return 0;
}

static struct file_operations chard_fops = {
        .owner = THIS_MODULE,
        .open = chard_open,
        .release = chard_release,
        .write = chard_write,
        .read = chard_read
};


static int __init chard_init(void)
{
    int result;
    if (dev_major) {
        dev = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(dev, n_drivers, "chard");
    } else {
        result = alloc_chrdev_region(&dev, dev_minor, n_drivers, "chard");
        dev_major = MAJOR(dev);
    }
    
    if (result < 0) {
        printk(KERN_WARNING "chard: can't get major %d\n", dev_major);
        return result;
    }

    cdev_init(&chard_cdev, &chard_fops);
    cdev_add(&chard_cdev, dev, 1);

    clase = class_create("char_class");
    dispositivo = device_create(clase, NULL, dev, NULL, "chard");

    printk(KERN_ALERT "Dispositivo insertado correctamente. Major: %d, Minor: %d\n", dev_major, dev_minor);
    
    return 0;
}

static void __exit chard_exit(void)
{
    device_destroy(clase, dev);
    class_destroy(clase);
    cdev_del(&chard_cdev);
    unregister_chrdev_region(dev, n_drivers);
    printk(KERN_ALERT "Dispositivo eliminado correctamente\n");
}
module_init(chard_init);
module_exit(chard_exit);