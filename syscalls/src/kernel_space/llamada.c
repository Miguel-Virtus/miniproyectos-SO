#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(llamada)
{
    printk("Saludos desde el espacio de kernel\n");
    return 0;
}
