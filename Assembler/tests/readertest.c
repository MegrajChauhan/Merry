#include "../includes/reader.h"
#include <stdio.h>

int main()
{
    Reader *reader = read_file("test.masm");
    if (reader == NULL)
        return -1;
    printf("Read:\n%s\n", reader->file_contents);
    destroy_reader(reader);
    return 0;
}