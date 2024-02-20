#include "../includes/reader.h"

size_t get_file_size(FILE *file)
{
    if (file == NULL)
        return 0;
    fseek(file, SEEK_SET, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    return size;
}

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
    // reader->filename = (char *)malloc(strlen(filepath));
    // if (reader->filename == NULL)
    // {
    //     fprintf(stderr, "Read Internal Error: This isn't your fault; There is problem in the internals.\n");
    //     free(reader);
    //     fclose(file);
    //     return NULL;
    // }
    reader->filename = filepath;
    size_t filelen = get_file_size(file);
    if (filelen == 0)
    {
        fprintf(stderr, "Read Error: The source file \"%s\" is empty; Cannot be empty.\n", filepath);
        destroy_reader(reader);
        fclose(file);
        return NULL;
    }
    reader->file_contents = (char *)malloc(filelen);
    // again redundant code
    if (reader->file_contents == NULL)
    {
        fprintf(stderr, "Read Internal Error: This isn't your fault; There is problem in the internals.\n");
        destroy_reader(reader);
        fclose(file);
        return NULL;
    }
    // now we read the file
    fread(reader->file_contents, 1, filelen, file); // shouldn't fail
    fclose(file);
    reader->iter.start = reader->file_contents;
    reader->iter.end = reader->file_contents + filelen;
    return reader;
}

void destroy_reader(Reader *reader)
{
    if (reader == NULL)
        return;
    if (reader->file_contents != NULL)
        free(reader->file_contents);
    // if (reader->filename != NULL)
    //     free(reader->filename);
    free(reader);
}