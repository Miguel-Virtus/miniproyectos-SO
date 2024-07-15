<div style="text-align: justify">

# Soluciones miniproyecto desarrollo de llamadas al sistema.

## Significado de `SYSCALL_DEFINE0` y `printk`

La mejor forma de definir llamadas al sistema es mediante las macros `SYSCALL_DEFINE#`, que se encargan de abstraer la creación de llamadas al sistema con # argumentos. La definición de estas macros se encuentra en `/linux/syscalls.h`. Estas macros reciben `2# + 1` parámetros. Primero el nombre de la llamada al sistema, y seguidamente el tipo y nombre de cada argumento.

 De momento, al no tener argumentos la llamada al sistema solo se introduce el nombre de esta.

por otro lado, `printk` es una función que imprime mensajes en el buffer de mensajes del kernel. Es una función muy útil para depurar el código del kernel, ya que permite imprimir mensajes en cualquier parte del código.

## `asmlinkage` y  prefijo `sys`

Con el modificador `asmlinkage`, se dice al compilador a modo de optimización que no busque los argumentos en los registros del procesador, sino en la pila del procesador.

El nombre de la función debera llevar el prefijo `sys_`, que se le añade en la expansión de la macro `SYSCALL_DEFINE0` al nombre que le pusimos a la llamada. Esto se debe a que es convención en el desarrollo del kernel que las llamadas al sistema comiencen por este prefijo, por lo que serán facilmente identificadas.


## Campos de `syscall_64.tbl`

El archivo se encuentra organizado por filas y columnas, donde cada fila representa una llamada al sistema y las columnas hacen referencia a un número identificativo de la llamada, la compatibilidad con otras arquitecturas (`64` `32` o `common`), el directorio donde se encuentra la llamada al sistema y el propio nombre de esta. Si leemos los comentarios del archivo en detalle, vemos como se nos índica que nuevas llamadas al sistema deberán ser añadidas después de la última de tipo common. En nuestra versión del kernel este número es 452.

Esta tabla es necesaria para que el kernel pueda identificar las llamadas al sistema y asignarles un número identificativo.

## Uso `menuconfig`

`menuconfig` es una herramienta que nos permite configurar el kernel de forma gráfica. Creando un archivod e configuración `.config` que se usará para compilar el kernel.

## significado `-j$(nproc)`
El parámetro `-j` indica el número de procesadores lógicos a utilizar durante la compilación. Para aprovechar al máximo los recursos del sistema utilizaremos el número total de procesadores que tengamos, que obtenemos con el comando `nproc`.


## Valor retorno de llamada al sistema
En caso de éxito, la llamada al sistema retorna `0`, se trata de una convención en el kernel de Linux. En caso de error, la llamada al sistema retornará un valor negativo.

## Posición de nuevo parámetro en `task_struct`
En nuestra versión del kernel el lugar donde añadir el nuevo campo queda definido por los comentarios del propio código fuente, prácticamente al final pero encima del campo struct thread_struct thread y de la macro randomized_struct_fields_end. Sitio donde se indica por otro comentario que es el lugar donde se deben añadir nuevos campos.

## Cambios en `SYSCALL_DEFINE2`
Cuenta con 4 parámetros más que SYSCALL_DEFINE0, los dos primeros identifican el tipo de la primera variable y su nombre (número que se va a escribir), y los dos segundos los de la segunda variable (PID del proceso).

## Uso de `for_each_process`
El kernel de linux guarda todos los procesos (task_struct) en una lista doblemente enlazada, existen macros que nos permiten iterar facilmente por dicha lista. Una de ellas es la macro for_each_process(X), que se expande a un bucle que itera sobre cada proceso del
sistema, poniendo en X el puntero a la estructura task_struct del proceso sobre el que se itera.

## variable con modificador __user y `copy_to_user`
El espacio de memoria en el que se ejecuta el kernel (y por lo tanto las llamadas al sistema) es diferente al que se ejecutan las aplicaciones de usuario, dejando de lado diferencias técnicas entre ellos, la principal diferencia es que las direcciones de memoria no son las mismas, por lo tanto, no son compatibles. Para poder comunicarse entre estos espacios será necesario utilizar las funciones `copy_to_user` y `copy_from_user`. La primera para pasar memoria del kernel al usuario, y la segunda para lo contrario.

El modificador `__user` indica que la variable es de espacio de usuario, por lo que no se puede acceder directamente a ella desde el kernel.

</div>