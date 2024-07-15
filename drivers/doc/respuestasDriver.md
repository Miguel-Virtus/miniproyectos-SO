<div style="text-align: justify">

# Soluciones miniproyecto desarrollo de drivers

## Uso de __init y __exit

En la declaración de las funciones previas, se incluyen los modificadores `__init` y `__exit`, que a pesar de parecer obligatorios no lo son. Estos *tokens*, de carácter optativo, sirven de ayuda al compilador, por ejemplo, en el caso de `__init` el compilador sabrá que esa función se utiliza únicamente en la creación del driver, por lo que puede eliminar de memoria el código de la misma, pues sabe que no se volverá a utilizar. De la misma forma pero de manera inversa, ocurre con el modificador `__exit`.

## Licencia

Existen varios tipos, en este caso hemos elegido la licencia Dual BSD/GPL, que hace referencia a una licencia BSD o GNU Public License v2. El tipo de licencia utilizada en este caso es meramente informativa, pero útil para que los usuarios conozcan el tipo de drivers que se encuentran en su sistema. 

Existen otros tipos de licencias, como por ejemplo la licencia MIT, o propietarias. Otras macros que se pueden utilizar son `MODULE_AUTHOR` o `MODULE_DESCRIPTION`, que sirven para describir el autor del módulo y una descripción del mismo, respectivamente.

## Funcionamiento del makefile

### Instrucción `make -C $KERNEL M=$PWD modules`
Esta invocación de la orden make indica que se debe cambiar al directorio especificado con -C antes de realizar cualquier acción. con M se indica a que directorio cambiar antes de comenzar la compilación.

Por lo tanto la instrucción anterior nos indica que cambiemos al directorio donde se encuentra el kernel, se tome el target *modules* del makefile que se encuentra en esa localización, pero que se ejecute de nuevo en el directorio original. Este target compilará todos los modulos que se encuentren en la variable obj-m, que en este caso solo hace referencia a hello.o.

### Archivo makefile complejo:

Su funcionamiento es realmente sencillo, este funcionará en que el archivo será llamado dos veces, desde dos sitios diferentes. Inicialmente será llamado desde la terminal. En este primer paso la variable KERNELRELEASE no habrá sido definida, por lo que se saltará a la segunda parte del programa, la cual invoca a make tal y como hacíamos desde la linea de comandos previamente.

En esta primera llamada, se ejecutará el Make del directorio del kernel pero en el  directorio de trabajo, que como hemos visto antes leera el Makefile en búsqueda de la variable obj-m. En este caso como sí se ha definido KERNELRELEASE, se entrará por el ifneq del principio obteniendo el valor de dicha variable.

## Registro de major y minor

Esta pareja de números se identifica en el kernel mediante el tipo de dato `dev_t`, y se puede crear con la macro `MKDEV(x,y)` a partir del major y el minor. El tipo de dato `dev_t` no es más que un entero sin signo (32 bytes) donde los primeros 12 bytes representan el major y el resto (20) el minor. Además, con las macros `MAJOR(x)` y `MINOR(x)` podemos encontrar el major y el minor de una variable de tipo `dev_t` respectivamente.

reservar los números major y minor se puede hacer de dos maneras, sabiendo el major, o dejando que el sistema nos asigne un major libre. en el primer caso se utilizará la función `register_chrdev_region(dev_t from, unsigned count, const char *name)`, que recibe el major y el minor en forma de `dev_t`, el número de dispositivos que se van a registrar (a partir del minor escogido, y en nuestro caso, 1) y el nombre del driver.

En caso de querer que se asigne dinamicamente el major, utilizaremos la función `alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)`, que recibirá un puntero a la variable dev_t, donde escribirá el valor, y podremos obtener el major con la macro MAJOR(x), también recibe el minor, el número de dispositivos a registrar y el nombre del driver.

En caso de éxito, ambas funciones devuelven 0, y en caso de error un número negativo. El fragmento de código relativo al registro del major y el minor es el definido en 4.3, que se encuentra en la función init del driver.

## Uso de `filp->private_data`
El campo `void *private_data` de la estructura file podremos alojar los valores que deseemos, Aquí guardaremos el puntero a la variable con el número generado aleatoriamente.

## Tipo de variable `rand`
Esta variable, al ser de 8 bits la podremos guardar en un `unsigned char` para que vaya de 0 a 255.

## Espacio de usuario y espacio de kernel
El espacio de memoria en el que se ejecuta el kernel es diferente al que se ejecutan las aplicaciones de usuario, dejando de lado diferencias técnicas entre ellos, la principal diferencia es que las direcciones de memoria no son las mismas, por lo tanto, no son compatibles. Para poder comunicarse entre estos espacios será necesario utilizar las funciones copy_to_user y copy_from_user. La primera para pasar memoria del kernel al usuario, y la segunda para lo contrario.

## Retorno de operación de lectura:
La función de lectura del driver devolverá tantos bytes como se hayan leído.

## Errores en linux, `ENOMEM`
Los errores en Linux se representan con un número negativo, cada error con un código distinto, por ejemplo el error de falta de memoria está definido con la macro `ENOMEM` que representa el número 12. A la hora de devolver este error por lo tanto se devolverá `-ENOMEM`.

## kmalloc y `GFP_KERNEL`
 indica la forma de obtener las páginas, de qué zonas se van a pedir las páginas o la importancia que tiene la solicitud. En nuestro caso y generalmente, utilizaremos el flag GFP_KERNEL, podemos encontrar otros flags en [este enlace](https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html).

 ## Significado `S_IRUGO`
Este flag indica que el archivo se puede leer por cualquier usuario, pero no permite escritura, es decir, es de solo lectura. En caso de querer que el archivo sea de lectura y escritura, se utilizaría `S_IRUGO | S_IWUGO`. Otros flags estan definidos en `include/linux/stat.h`.

## Uso `get_user` y `put_user`
Son utilizados cuando el paso de memoria es de variables simples. Ya que en este caso vamos a pasar byte a byte la información haremos uso de estas macros.

Para el paso de memoria de usuario a kernel se utiliza get_user(x,y), cuyos parámetros son la variable en la que guardar el valor (en espacio de kernel) y un puntero a una variable en espacio de usuario que apunte al valor que se desea copiar.

En la dirección contraria, put_user(x,y) funciona de forma similar, pasando la variable que se va a copiar en el primer parámetro, y la dirección en espacio de usuario en el segundo.

## Resultado final
El código final del driver del buffer circular se encuentra en [este archivo](../src/ring.c).

</div>