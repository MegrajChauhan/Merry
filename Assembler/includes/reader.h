#ifndef _READER_
#define _READER_

#include <stdio.h>
#include <stdlib.h>
#include "../utils/strutils.h"

typedef struct Reader
{
    char *file_contents; // the contents of the file
    char *filename;      // file name
} Reader;

Reader *read_file(char *filepath);

#endif