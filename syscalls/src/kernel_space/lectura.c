#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

SYSCALL_DEFINE2(llamada, int, pid, int __user *, addr)
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
