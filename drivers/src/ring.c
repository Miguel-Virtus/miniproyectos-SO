#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> 
#include <linux/random.h>
//#include <linux/uaccess.h> // copy to user
#include <asm/uaccess.h> // copy to user
#include <linux/slab.h> // kmalloc kfree

unsigned int SIZE = 16;
module_param(SIZE, uint, S_IRUGO);
MODULE_LICENSE("Dual BSD/GPL");

int dev_major = 0;
int dev_minor = 0;

unsigned int n_drivers = 1;

dev_t dev;
struct cdev ring_cdev;
struct class * clase;
struct device * dispositivo;

struct ring_dev_buffer {
    char *buf;
    int inicio;
    int final;
    unsigned int escrito;
};

struct ring_dev_buffer ring_buf;

static int ring_open(struct inode *inode, struct file *filp) {
    printk("Abierto\n");
    return 0;
}

static int ring_release(struct inode *inode, struct file *filp) {
    printk("Cerrado\n");
    return 0;
}

static ssize_t ring_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_ALERT "Solicitada escritura de %ld dígitos", count);

    if (count > (SIZE - ring_buf.escrito)) {
        count = SIZE - ring_buf.escrito;
    }

    printk(KERN_ALERT "Escribiendo %ld", count);
    for (int d = 0; d < count; d++) {
        get_user(ring_buf.buf[ring_buf.final], buf + d);
        ring_buf.final = (ring_buf.final + 1) % SIZE;
        ring_buf.escrito++;
    }

    printk("Inicio: %d", ring_buf.inicio);
    printk("Final: %d", ring_buf.final);
    printk("Escrito: %d", ring_buf.escrito);

    if (ring_buf.inicio <= ring_buf.final) {
        for (int i = 0; i < ring_buf.escrito; i++) {
        printk("%c", ring_buf.buf[ring_buf.inicio + i]);
        }
    }
    else {
        for (int i = ring_buf.inicio; i < SIZE; i++) {
            printk("%c", ring_buf.buf[i]);
        }
        for (int i = 0; i < ring_buf.final; i++) {
            printk("%c", ring_buf.buf[i]);
        }
    }
    
    if (count == 0) {
        return -ENOMEM;
    }
    return count;
}

static ssize_t ring_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_ALERT "Solicitada lectura de %ld dígitos", count);

    if (count > ring_buf.escrito) {
        count = ring_buf.escrito;
    }

    printk(KERN_ALERT "Escribiendo %ld", count);

    for (int d = 0; d < count; d++) {
        put_user(ring_buf.buf[ring_buf.inicio], buf + d);
        ring_buf.inicio = (ring_buf.inicio + 1) % SIZE;
        ring_buf.escrito--;
    }



    printk("Inicio: %d", ring_buf.inicio);
    printk("Final: %d", ring_buf.final);
    printk("Escrito: %d", ring_buf.escrito);

    if (ring_buf.inicio <= ring_buf.final) {
        for (int i = 0; i < ring_buf.escrito; i++) {
        printk("%c", ring_buf.buf[ring_buf.inicio + i]);
        }
    }
    else {
        for (int i = ring_buf.inicio; i < SIZE; i++) {
            printk("%c", ring_buf.buf[i]);
        }
        for (int i = 0; i < ring_buf.final; i++) {
            printk("%c", ring_buf.buf[i]);
        }
    }
    return count;
}


static struct file_operations ring_fops = {
        .owner = THIS_MODULE,
        .open = ring_open,
        .release = ring_release,
        .write = ring_write,
        .read = ring_read
};


static int __init hello_init(void)
{
    int result;
    if (dev_major) {
        dev = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(dev, n_drivers, "ring");
    } else {
        result = alloc_chrdev_region(&dev, dev_minor, n_drivers, "ring");
        dev_major = MAJOR(dev);
    }
    
    if (result < 0) {
        printk(KERN_WARNING "ring: can't get major %d\n", dev_major);
        return result;
    }

    cdev_init(&ring_cdev, &ring_fops);
    cdev_add(&ring_cdev, dev, 1);

    clase = class_create("ring_class");
    dispositivo = device_create(clase, NULL, dev, NULL, "ring");

    ring_buf.buf = (char *) kmalloc(SIZE * sizeof(char), GFP_KERNEL);
    ring_buf.inicio = 0;
    ring_buf.final = 0;
    ring_buf.escrito = 0;
    printk(KERN_ALERT "Dispositivo insertado correctamente. Major: %d, Minor: %d\n", dev_major, dev_minor);
    return 0;
}

static void __exit hello_exit(void)
{
    kfree(ring_buf.buf);
    device_destroy(clase, dev);
    class_destroy(clase);
    cdev_del(&ring_cdev);
    unregister_chrdev_region(dev, n_drivers);
    printk(KERN_ALERT "Dispositivo eliminado correctamente\n");
}
module_init(hello_init);
module_exit(hello_exit);