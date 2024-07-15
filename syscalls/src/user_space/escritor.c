#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define __NR_lectura 452

#define __NR_escritura 453

long escritura_syscall(int numero, int pid)
{
    return syscall(__NR_escritura, numero, pid);
}

int main(int argc, char *argv[])
{
    int pid, valor;
    printf("Introduce pid: ");
    scanf("%d", &pid);
    printf("introduce valor: ");
    scanf("%d", &valor);

    long activity = escritura_syscall(valor, pid);
    
    if (activity >= 0) {
        printf("exito en la escritura\n");
    } else {
        perror("fallo en la escritura\n");
    }

    return 0;
}

