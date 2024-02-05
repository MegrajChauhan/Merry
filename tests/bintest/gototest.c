#include <stdio.h>
// 9813143403
int x(int p)
{
    int ret = 0;
    if (p == 0)
        ret++;
_here_:
    ret++;

    return ret;
}

int main()
{
    printf("ret = %d\n", x(0));
}