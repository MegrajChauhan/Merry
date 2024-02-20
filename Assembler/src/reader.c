#include "../includes/reader.h"

Reader *read_file(char *filepath)
{
    if (ends_with(filepath, ".masm") != 0)
        return NULL;
    Reader *reader = (Reader *)malloc(sizeof(Reader));
    if (reader == NULL)
        return NULL;
    FILE *file = fopen(filepath, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Read Error: Unable to open file \"%s\". Could be a directory or doesn't exists.\n", filepath);
        free(reader);
        return NULL;
    }
    
}