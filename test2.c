#include <stdio.h>
#include <unistd.h>
#include <errno.h>

char foo[] = "Foo";
char bar[100];

int main(void)
{
    int ret = syscall(472, foo, 3, bar, 100);
    printf("ret: %d\n%s\n", ret, bar);
    if (ret < 0)
        perror("syscall error:");
    return 0;
}
