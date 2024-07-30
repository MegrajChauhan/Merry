#include <stdio.h>
#include <string.h>

#define SIZE 48

unsigned long hash(char *str, size_t len) {
    unsigned long h = 5381;  // Starting value (a large prime number is often used)
    for (size_t i = 0; i < len; i++) {
        h = ((h << 5) + h) + str[i]; // h * 33 + str[i]
    }
    return h % SIZE;
}

int main()
{
    char strings[23][50] = {
        "aBcDeFgHiJkLmNoPqRsTuVwXyZ",
        "abc",
        "xyz",
        "1234567890",
        "12z4567890",
        "qwertyuiop",
        "asdfghjkl",
        "zxcvbnm",
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM",
        "RandomString1",
        "RandomString2",
        "RandomString3",
        "RandomString4",
        "RandomString5",
        "LoremIpsum",
        "DolorSitAmet",
        "Consectetur",
        "AdipiscingElit",
        "SedDoEiusmod",
        "TemporIncididunt",
        "UtLaboreEtDolore"};
    for (size_t i = 0; i < 20; i++)
    {
        printf("%s: %lu\n", strings[i], hash(strings[i], strlen(strings[i])));
    }
}