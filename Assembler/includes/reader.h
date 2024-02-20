#ifndef _READER_
#define _READER_

#include <stdio.h>
#include <stdlib.h>
#include "../utils/strutils.h"
#include "../utils/iters.h"

typedef struct Reader
{
    char *file_contents; // the contents of the file
    char *filename;      // file name
    size_t file_len;
    Iter iter;
} Reader;

Reader *read_file(char *filepath);

void destroy_reader(Reader *reader);

#endif