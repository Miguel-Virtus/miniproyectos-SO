#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(escritura, int, numero, int, pid)

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
