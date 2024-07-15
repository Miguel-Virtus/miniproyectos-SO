<div style="text-align: justify">

# Programación de dispositivos en Linux

En este documento se recoge paso a paso el proceso de creación de drivers de dispositivos de carácter para el kernel de Linux.

## Proceso de trabajo

### Pasos previos
> Este documento ha sido realizado utilizando Ubuntu 22.04 y la versión de kernel 6.5

1. Localice la versión del kernel que está utilizando con `uname -r`, y defina la variable de entorno `KERNEL` con el directorio donde se encuentra el código fuente, que será `/lib/modules/$(uname -r)/build`.


### Estructura básica de un driver

1. Cree una carpeta y en ella un archivo `hello.c` con el siguiente contenido:

```c
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int __init init_driver(void)
{
    printk(KERN_ALERT "Hola mundo\n");
    return 0;
}
static void __exit exit_driver(void)
{
    printk(KERN_ALERT "Adiós mundo\n");
}
module_init(init_driver);
module_exit(exit_driver);
```

>Las funciones que se ejecutan al dar de alta/baja un driver son `init_driver` y `exit_driver`, definidas en el programa. Pero para que el sistema sepa al compilar el driver, que función es cual, se especifica a partir de las macros `module_init` y `module_exit`

> En estas funciones aparecen los modificadores `__exit` e `__init`. Investigue acerca del uso de estos tokens, ¿Son obligatorios? ¿Cuál es su función?

> La macro `MODULE_LICENSE` especifica, a modo de información, la licencia con la que cuenta el driver. ¿Qué otros tipos de licencias existen? ¿Que ortas macros pueden aportar información sobre el driver?

2.  Una vez creado el archivo fuente, cree un `Makefile` con el siguiente contenido:

```
obj-m := hello.o
```

> La variable `obj-m` indica los drivers que deben ser compilados, con esta línea le hacemos saber que debe compilar el driver que acbamos de crear.

3. Situado en el directorio que contiene tanto el `Makefile` como `hello.c`, compile el driver con:

```console
user@pc$ make -C $KERNEL M=$PWD modules
```

> Es posible que deba cambiar la versión de gcc instalada para que coincida con la del kernel.

4. Se generará un archivo `hello.ko` con el driver compilado. Pruebe a insertarlo con `sudo insmod hello.ko` y a eliminarlo con `sudo rmmod hello.ko`.

5. Puede ver los mensajes que escribe el programa con `printk` en el buffer de log del kernel, para ello ejecute `sudo dmesg` y localícelos.

### Automatización de archivo Makefile

1. Para automatizar el proceso de compilación del driver, modifique el archivo `Makefile` para que tenga la siguiente estructura:

```
ifneq ($(KERNELRELEASE),)
    obj-m := hello.o
else
default:
    $(MAKE) -C $(KERNEL) M=$(PWD) modules
endif
```

> ¿Qué cree que hace este `Makefile`?, ¿Qué indica `-C`? ¿y `M`?

### Creación de dispositivo de tipo carácter básico

A continuación vamos a realizar un dispositivo de tipo de carácter, que funciona como un flujo de datos de entrada y salida. Antes de comenzar, investigue que otros tipos de dispositivos se podrian crear.

1. Copie el archivo `hello.c` a `chard.c`, sobre el que realizaremos los siguientes cambios:

2. Los dispositivos de carácter cuentan con una estructura `file_operations` que contiene las funciones que se ejecutan al realizar operaciones sobre el dispositivo. Cree una variable de tipo `struct file_operations` y añada las siguientes funciones:


```c
static int chard_open(struct inode *inode, struct file *filp) {
printk("Abierto\n"); return 0;
}
static int chard_release(struct inode *inode, struct file *filp) {
printk("Cerrado\n"); return 0;
}
static ssize_t chard_write(struct file *filp, const char __user *buf, size_t count,
loff_t *f_pos) {
printk("Escrito\n"); return count;
}
static ssize_t chard_read(struct file *filp, char __user *buf, size_t count, loff_t
*f_pos) {
printk("Leido\n"); return count;
}
static struct file_operations chard_fops = {
.owner = THIS_MODULE,
.open = chard_open,
.release = chard_release,
.write = chard_write,
.read = chard_read
};
```


3. De momento no entraremos en detalle en el funcionamiento de estas funciones, simplemente imprimirán un mensaje en el buffer de log del kernel cuando se realice la operación correspondiente.

4. Los dispositivos se identifican mediante dos números, major y minor. El major indica el manejador que maneja al dispositivo (un manejador puede manejar varios dispositivos) y el minor indica el dispositivo concreto. La pareja de números se define en el kernel como un dato te tipo `dev_t`. Para registrar un major y un minor primero defina las siguientes variables:

```c
int dev_major = 0;
int dev_minor = 1;
unsigned int n_drivers = 1;
dev_t dev;
```

5. Posteriormente, en la función definida en `init_driver` añada el siguiente código:


```c
if (dev_major) {
    dev = MKDEV(dev_major, dev_minor);
    result = register_chrdev_region(dev, 1, "chard");
} else {
    result = alloc_chrdev_region(&dev, dev_minor, 1, "chard");
    dev_major = MAJOR(dev);
}
if (result < 0) {
    printk(KERN_WARNING "chard: can’t get major %d\n", dev_major);
    return result;
}
```

> ¿Para que sirve la función `register_chrdev_region` y `alloc_chrdev_region`? ¿Cuál es la diferencia entre ambas? ¿Qué es `MKDEV` y `MAJOR`?


6. A continuación se debe crear el dispositivo como tal, para ello defina las siguientes variables:

```c
struct cdev chard_cdev;
struct class * clase;
struct device * dispositivo;
```

7. Y añada el siguiente código en la función definida en `init_driver`:
8. 
```c
cdev_init(&chard_cdev, &chard_fops);
cdev_add(&chard_cdev, dev, 1);

clase = class_create("char_class");
dispositivo = device_create(clase, NULL, dev, NULL, "chard");
```

8. En la función definida en `exit_driver` tendremos que deshacer las acciones anteriores, añada el siguiente código:

```c
device_destroy(clase, dev);
class_destroy(clase);
cdev_del(&chard_cdev);
unregister_chrdev_region(dev, n_drivers);
```

### Implementación de las operaciones de apertura y lectura
En este apartado vamos a implementar las funciones de apertura y lectura del dispositivo. Vamos a desarrollar un driver que al abrirlo genere un número aleatorio de 8 bits y al leerlo devuelva una secuencia de números consecutivos.
copie el contenido de `chard.c` a `seq.c` y realice los siguientes cambios:

1. En la de apertura (`seq_open`) añada el siguiente código:

```c
static int seq_open(struct inode *inode, struct file *filp) {
    printk("Abierto\n");
    get_random_bytes(&rand, sizeof(rand));
    filp->private_data = &rand;
    printk("Generado número %u\n", rand);
    return 0;
}
```

> El parámetro `filp` es un puntero a una estructura contiene información sobre el archivo que se está abriendo. ¿Qué se está guardando en el campo `private_data`?

> Defina previamente la variable `rand`, ¿De qué tipo debe ser para almacenar un número de 8 bits?

1. La función de lectura `seq_read`, cuenta con 4 parámetros, el primero, `filp`, es idéntico al de la función anterior. `buf` es un puntero al buffer donde escribir los datos que se lean, y `count` el número de bytes que se solicitan leer. `f_pos` indica la posición de lectura, que en este driver no utilizaremos. Añada el siguiente código:

```c
static ssize_t seq_read(struct file *filp, char __user *buf, size_t count, loff_t *
f_pos) {
    printk("Solicitada una lectura de %ld bytes\n", count);
    unsigned char *c_ptr = filp->private_data;
    for (int i = 0; i < count; i++) {
        if (copy_to_user(buf+i, c_ptr, sizeof(*c_ptr)) != 0) {
            return -ENOMEM;
        }
        (*c_ptr)++;
    }
    filp->private_data = c_ptr;
    return count;
}
```

> La función `copy_to_user` es necesaria para copiar datos del espacio de kernel al espacio de usuario. ¿Por qué no se puede copiar directamente? ¿Cuál es la diferencia entre espacio de usuario y de kernel? ¿Qué significa el valor de retorno de esta función?

> En caso de que ocurra un error al copiar los datos, Se devuelve `-ENOMEM`. ¿Qué significa este valor?

### Desarrollo avanzado del driver:

Finalmente vamos a desarrollar un driver que funcione como un buffer circular. Para ello, copie el contenido de `seq.c` a `ring.c` e implemente las funciones de escritura y lectura.

1. Para alojar memoria de forma dinámica en el kernel, utilice la función `kmalloc`, y para desalojarla, `kfree`. A diferencia de `malloc`, `kmalloc` recibe en su segundo parámetro la forma en la que se va a utilizar la memoria, que en este caso será `GFP_KERNEL`. Investigue que significa este parámetro y que otros valores se pueden utilizar.

2. Defina una variable `SIZE` al principio del archivo que indique el tamaño del buffer, que será pasada posteriormente a `kmalloc`, inicialicela a un valor de 16.

3. Despues de la definición de la variable inserte `module_param(SIZE, int, S_IRUGO);` para que el tamaño del buffer pueda ser modificado al cargar el driver, investigue acerca del significado de `S_IRUGO` y otros valores que se pueden utilizar.

4. Defina una estructura que contenga el buffer y los punteros que sean necesarios para su funcionamiento. Aloje la memoria de forma dinámica en la función de inserción del driver y desalójela en la de eliminación.

5. Desarrolle las funciones de escritura y lectura, de forma que el dispositivo funcione de forma circular.

6. Para el paso de memoria entre el espacio de usuario y el de kernel, utilice las macros `put_user` y `get_user`. Investigue su funcionamiento.

7. Compile el driver y pruebe su funcionamiento.

## Cómo seguir

En esta sección se recogen una serie de enlaces que pueden ser de utilidad para continuar con el desarrollo de drivers de dispositivos en Linux:

- [Linux Device Drivers](https://lwn.net/Kernel/LDD3/): Por muchos considerada la biblia de los drivers de Linux. Este documento recoge de forma resumida los 3 primeros capítulos. En el libro se profundiza en el desarrollo de drivers de dispositivos en Linux, no solo de carácter, sino también de bloques y de red.

- [Programación de drivers de bloques](https://linux-kernel-labs.github.io/refs/heads/master/labs/block_device_drivers.html): Documento que explica cómo desarrollar drivers de bloques en Linux.

- [Recopilación de fuentes para programación a bajo nivel](https://github.com/gurugio/lowlevelprogramming-university): Se trata de una recopilación de libros, webs y proyectos relacionados con la programación a bajo nivel.

- [Miniproyectos relacionados con los Sistemas Operativos](https://github.com/forestLoop/Learning-EI338): Recopilación de miniproyectos relacionados con Sistemas Operativos, desarrollo de módulos, Shell de Unix, gestión de procesos, memoria y memoria virtual.
## Respuestas
Las respuestas a los conceptos que se preguntan en este documento estan recogidos en el archivo de [respuestas](respuestas.md)

</div>