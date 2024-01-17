#include <stdio.h>

typedef struct
{
    int pqr;
    int xyz;
} temp;

void assign(int *ptr)
{
    int testval = 101;
    *ptr = testval;
}

void assignstr(temp *ptr)
{
    temp new = {33, 44};
    *ptr = new;
}

int main()
{
    int a = 10;
    int *x = &a;
    assign(x);
    printf("VALUE IS: %d\n", *x);
    temp t = {11, 22};
    temp *ptr = &t;
    assignstr(ptr);
    printf("After assignstr: %d and %d\n", t.pqr, t.xyz);
    printf("After assignstr deref: %d and %d\n", ptr->pqr, ptr->xyz);
}