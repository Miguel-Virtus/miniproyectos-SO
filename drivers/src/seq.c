#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

int dev_major = 0;
int dev_minor = 0;

unsigned int n_drivers = 1;

dev_t dev;
struct cdev seq_cdev;
struct class * clase;
struct device * dispositivo;

unsigned char rand;

static int seq_open(struct inode *inode, struct file *filp) {
    printk("Abierto\n");
    get_random_bytes(&rand, sizeof(rand));
    filp->private_data = &rand;
    printk("Generado número %u\n", rand);
    return 0;
}

static int seq_release(struct inode *inode, struct file *filp) {
    printk("Cerrado\n");
    return 0;
}

static ssize_t seq_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    printk("Escrito\n");
    return count;
}

static ssize_t seq_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    printk("Solicitada una lectura de  %ld bytes\n", count);
    unsigned char *c_ptr = filp->private_data;
    for (int i = 0; i < count; i++) {
        if (copy_to_user(buf+i, c_ptr, sizeof(*c_ptr)) != 0) {
            return -ENOMEM;
        }
        (*c_ptr)++;
    }
    filp->private_data =  c_ptr;
    return count;
}


static struct file_operations seq_fops = {
        .owner = THIS_MODULE,
        .open = seq_open,
        .release = seq_release,
        .write = seq_write,
        .read = seq_read
};


static int __init hello_init(void)
{
    int result;
    if (dev_major) {
        dev = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(dev, n_drivers, "seq");
    } else {
        result = alloc_chrdev_region(&dev, dev_minor, n_drivers, "seq");
        dev_major = MAJOR(dev);
    }
    
    if (result < 0) {
        printk(KERN_WARNING "seq: can't get major %d\n", dev_major);
        return result;
    }

    cdev_init(&seq_cdev, &seq_fops);
    cdev_add(&seq_cdev, dev, 1);

    clase = class_create("seq_class");
    dispositivo = device_create(clase, NULL, dev, NULL, "seq");

    printk(KERN_ALERT "Dispositivo insertado correctamente. Major: %d, Minor: %d\n", dev_major, dev_minor);
    return 0;
}
static void __exit hello_exit(void)
{


    device_destroy(clase, dev);
    class_destroy(clase);
    cdev_del(&seq_cdev);
    unregister_chrdev_region(dev, n_drivers);
    printk(KERN_ALERT "Dispositivo eliminado correctamente\n");
}
module_init(hello_init);
module_exit(hello_exit);