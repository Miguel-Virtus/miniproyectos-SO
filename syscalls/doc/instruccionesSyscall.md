<div style="text-align: justify">

# Programación de llamadas al sistema en Linux


En este documento se recoge paso a paso el proceso de creación llamadas al sistema en el kernel de linux.

## Proceso de trabajo

### Pasos previos:

1. Descarge el kernel de linux desde la página oficial de [kernel.org](https://mirrors.edge.kernel.org/pub/linux/kernel/), búsque la versión que desee, descargue el archivo y extraigalo en la carpeta de su preferencia. Es recomendable instalar una versión similar a la que tiene su sistema operativo, puede verificar la versión de su kernel con el comando `uname -r`. Anote el resultado de este comando para comprobar en el futuro que el miniproyecto se ha realizado correctamente.

> En este desarrollo se ha utilizado Ubuntu 22.04 con la versión 6.5 del kernel de linux.


2. Instale las herramientas necesarias para la compilación del kernel, que son las siguientes:

```bash
$ sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```

### Creación de llamada al sistema básica

1. Dentro del código fuente del kernel, cree una carpeta con el nombre que desee, y dentro un archivo con extensión `.c` que contendrá la implementación de la llamada al sistema. Escriba en el archivo el siguiente código:

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(llamada)
{
    printk("Saludos desde el espacio de kernel\n");
    return 0;
}
```

> ¿Qué función realiza `SYSCALL_DEFINE0`? ¿Qué hace `printk`?

2. A continuación, cree un archivo `Makefile` en el mismo directorio con el siguiente contenido:

```makefile
obj-y := llamada.o
```

3. Para que la llamada sea compilada, tendremos que añadir este directorio al sistema de compilación del kernel. Añada al final del archivo Kbuild del directrio raíz del kernel la siguiente línea:

```makefile
obj-y += llamada/
```

> En versiones anteriores es posible que el proceso sea diferente, por lo que deberá en vez de añadir esa línea al final del archivo `Kbuild` entrar en el archivo `Makefile` del directorio raíz y añadir `llamada/` a la variable `core-y`.

3. Añada la cabecera de la llamada al sistema en el archivo `include/linux/syscalls.h`:

```c
asmlinkage long sys_llamada(void);
```

> ¿Qué significa `asmlinkage`? ¿Por qué el nombre de la función es `sys_llamada` y no `llamada`?

4. Abra el archivo `arch/x86/entry/syscalls/syscall_64.tbl` y localice, leyendo los comentarios del archivo, la posición en la que debe añadir la llamada al sistema. En nuestro caso creamos la siguiente entrada:

```c
<numero>     common      llamada                 sys_llamada
```

> ¿Qué número se debe poner? ¿Qué significa cada campo de la entrada? ¿Por qué se añade la llamada al sistema en esta tabla?

5. Anote el número y vuelva al directorio raíz, vamos a compilar el kernel. Para ello ejecute el siguiente comando.

```bash
$ make menuconfig
```

6. En el menú que aparece, no realice ningún cambio, simplemente guarde y salga.


> ¿Qué hace este comando? ¿Por qué es necesario ejecutarlo? Investigue su el contenido del menú que se muestra.


> Es posible que en versiones de Ubuntu/Debian deba borrar el contenido de los campos `Additional X.509 keys for default system keyring` y `X.509 certificates to be preloaded into the system blacklist keyring` en el menú `Cryptographic API -> Certificates for signature checking`, en caso de que durante el proceso de compilación se muestre un error realcionado con certificados.


7. Compile el kernel, para ello aproveche el número de núcleos de su procesador para acelerar el proceso:

```bash
$ make -j$(nproc)
```

> ¿Cual es el significado de `-j$(nproc)`? 


8. Una vez compilado el kernel, instale los módulos y el kernel en su sistema:

```bash
$ sudo make modules_install -j$(nproc)
$ sudo make install -j$(nproc)
```

9. Depende de la distribución que esté utilizando, deberá actualizar el gestor de arranque. En nuestro caso, utilizamos `grub`:

```bash
$ sudo update-grub
```

1.  Reinicie el sistema, en el menú de arranque seleccione la nueva versión del kernel que ha compilado. Para mostrar el menú de arranque, mantenga pulsada la tecla <kbd>shift</kbd> o <kbd>esc</kbd> durante el arranque según si su sistema es BIOS o UEFI respectivamente.

2.  Una vez iniciado el sistema, abra una terminal y ejecute el siguiente comando:

```bash
uname -r
```

12. Si el resultado es la versión del kernel que ha compilado, la llamada al sistema ha sido creada correctamente. Para probarla, cree en otro directorio un archivo `prueba.c` con el siguiente contenido:

```c
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    long int resultado = syscall(452);
    printf("Valor devuelto por la llamada al sistema: %ld\n", resultado);
    return 0;
}
```

13. Compile el archivo y ejecutelo. Además, para ver los mensajes que se imprimen con `printk`, ejecute `dmesg`. Compruebe que todo ha funcionado correctamente.

```bash
$ gcc prueba.c -o prueba
$ ./prueba
$ dmesg
```

> ¿Que valor devuelve la llamada al sistema? ¿Por qué?


### Creación de par de llamadas al sistema más complejas

A continuación crearemos un par de llamadas al sistema que lean/escriban sobre un nuevo campo de cada proceso, que será identificado mediante su PID. Para ello, seguiremos los siguientes pasos:

1. Localice el archivo `/include/linux/sched.h` y dentro de él la estructura `task_struct`. Añada un nuevo campo a la estructura de tipo `int` llamado `special_number`. Investigue en qué lugar de la estructura debería añadir este campo atendiendo a los comentarios del archivo.

2. Repita el proceso anterior de creación de llamadas al sistema, creando dos nuevos directorios para dos nuevas llamadas. La llamada encargada de realizar la escritura tendrá el siguiente código:

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(escritura, int, numero, pid_t, pid)
{
	struct task_struct *p;
	for_each_process(p) {
		if (p->tgid == pid) {
			p->special_number = numero;
			printk("Escrito número %d.\n", numero);
		}
	}
	return 0;
}
```

> Entienda el código, ¿Qué cambios han ocurrido en la macro que define la llamada al sistema? ¿Qué hace `for_each_process`?


1. La llamada encargada de realizar la lectura tendrá el siguiente código:

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

SYSCALL_DEFINE2(llamada, pid_t, pid, int __user *, addr)
{
	struct task_struct *p;
	for_each_process(p) {
		if (p->tgid == pid) {
			printk("Mi número especial es: %d.\n", current->special_number);
			const int desde = current->special_number;
			copy_to_user(addr, &desde, sizeof(int));
		}
	}
	return 0;
}
```

> Entienda el código, ¿Qué significa que una variable tenga el modificador __user? ¿Qué hace `copy_to_user`?

4. Añada las entradas correspondientes en la tabla de llamadas al sistema y en el archivo `syscalls.h`, vuelva a compilar el kernel y reinicie el sistema.

5. Pruebe las llamadas al sistema de la siguiente forma:
	1. Cree en un nuevo directorio dos archivos `escritura.c` y `lectura.c` y codifiquelos de la siguiente manera:
		1. El archivo de escritura deberá leer por entrada estándar 2 números, que serán el número a escribir y el PID del proceso al que se le escribirá. Con estos datos, invocará la llamada al sistema con los parámetros leídos.
   		2. El archivo de lectura invocará una primera vez a la llamada al sistema de lectura con su propio PID, y mostrará por pantalla tanto el valor leído como su PID. A continuación se bloqueará hasta recibir una señal (por ejemplo con un `scanf`) y a continuación volverá a realizar la llamada al sistema de lectura con su PID, mostrando de nuevo el valor leído y su PID.
   3. Compile ambos archivos y ejecútelos en dos terminales diferentes. Cambie el valor del campo del proceso de lectura con el proceso de escritura y compruebe que el cambio ocurre de forma correcta.

Pruebe a codificar estos programas. ¿Cómo habría que pasar los argumentos a las llamadas al sistema? Si se atasca, puede ver una solución [aquí](../src/user_space/).

## Cómo seguir

En esta sección se recogen recursos adicionales para profundizar en la creación de llamadas al sistema en Linux.

- [Linux Kernel Development](https://www.doc-developpement-durable.org/file/Projets-informatiques/cours-&-manuels-informatiques/Linux/Linux%20Kernel%20Development,%203rd%20Edition.pdf): Libro de Robert Love que explica en profundidad el desarrollo del kernel de linux, incluyendo la creación de llamadas al sistema.

- [Adding A System Call To The Linux Kernel (5.8.1) In Ubuntu (20.04 LTS)](https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8) : Artículo que explica cómo añadir una llamada al sistema en Ubuntu 20.04, y en el que se basa este documento.

- [Recopilación de fuentes para programación a bajo nivel](https://github.com/gurugio/lowlevelprogramming-university): Se trata de una recopilación de libros, webs y proyectos relacionados con la programación a bajo nivel.

## Respuestas

En [este](./respuestasSyscall.md) archivo se recogen las respuestas a las preguntas planteadas en este documento.

</div>