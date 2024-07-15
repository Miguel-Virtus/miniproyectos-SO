#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define __NR_lectura 452

#define __NR_escritura 453

int lectura_syscall(int pid)
{
    int resultado;
    syscall(__NR_lectura, pid, &resultado);
    return resultado;
}


int main(int argc, char *argv[])
{
    int activity = lectura_syscall(getpid());

    printf("exito en la primera lectura, leído: %d\n", activity);


    printf("Mi pid: %d\n", getpid());
    scanf("%d", &activity);
    printf("continuamos\n");
    activity = lectura_syscall(getpid());
    
    printf("exito en la primera lectura, leído: %d\n", activity);

    return 0;
}

