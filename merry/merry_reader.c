#include "internals/merry_reader.h"

_MERRY_INTERNAL_ mbool_t merry_check_file_extension(mcstr_t _file_name)
{
    if (surelyF(_file_name == NULL || strlen(_file_name) < 5))
        goto ret_err;
    // now check for the file extension
    mstr_t dot = strchr(_file_name, '.');
    if (surelyF(dot == NULL))
        goto ret_err;
    if (surelyT(strcmp(dot + 1, "mbin") == 0))
        return mtrue;
    goto ret_err;
ret_err:
    _READ_ERROR_("Read Error: The file %s is not a valid filename.\n", _file_name);
    return mfalse;
}

_MERRY_INTERNAL_ msize_t merry_get_file_len(FILE *f)
{
    msize_t len = 0;
    if (fseek(f, SEEK_SET, SEEK_END) != 0)
    {
        return 0; // this tells the caller that the file is empty when in fact there was problem calculating the size
    }
    len = ftell(f); // get the size
    rewind(f);      // rewind the stream
    return len;
}

_MERRY_INTERNAL_ mret_t merry_read_file_contents(MerryInpFile *inp)
{
    // get the length of the input file
    msize_t file_len = merry_get_file_len(inp->f);
    if (file_len == 0)
    {
        _READ_DIRERROR_("Read Error: The input file is empty. Expected to start with '['.\n");
        return RET_FAILURE;
    }
    inp->_file_contents = (mstr_t)merry_lalloc(file_len);
    if (inp->_file_contents == RET_NULL)
    {
        _READ_DIRERROR_("Read Internal Error: Failed to read the input file.\n");
        return RET_FAILURE;
    }
    if (surelyF(fread(inp->_file_contents, 1, file_len, inp->f) != 0))
    {
        merry_lfree(inp->_file_contents, file_len);
        _READ_DIRERROR_("Read Internal Error: Failed to read the input file.\n");
        return RET_FAILURE;
    }
    inp->iter = inp->_file_contents;
    return RET_SUCCESS; // we read the file successfully
}

_MERRY_INTERNAL_ mret_t merry_reader_parse_attributes(MerryInpFile *inp)
{
    // We will just read from [ till ]
    // we reach this point after reading the file
    if (*inp->iter != '[')
    {
        _READ_ERROR_("Read Error: Invalid syntax: Expected '[' at the beginning of the given input file '%s'.\n", inp->_file_name);
        return RET_FAILURE;
    }
    inp->iter++;
    mbool_t fmt_provided = mfalse, dlen_provided = mfalse, ilen_provided = mfalse;
    mbool_t ibstart_provided = mfalse, ibend_provided = mfalse, dbstart = mfalse, dbend = mfalse;
    mbool_t end_reached = mfalse;
    while (*inp->iter != ']' && *inp->iter != '\0')
    {
        // until we reach ']' we read and see if we
        // since the order of these attributes doesn't matter, we can continue parsing them
        char x = *inp->iter;
        if (x == ' ' || x == '\n' || x == '\t')
        {
            // there may be more escape sequence characters that we may want to skip
            while (x == ' ' || x == '\n' || x == '\t')
            {
                // we skip all of these characters
                inp->iter++;
                x = *inp->iter;
            }
        }
        else if (x == '.')
        {
            // we have an attribute to ourselves
            // we read the attribute and see what that attribute is
            inp->iter++;
            if (*inp->iter != '\0')
            {
                // we use the first character of the attribute to see what the attribute is
                switch (*inp->iter)
                {
                case 'f':
                {
                    // the file format is being specified
                    inp->iter++;
                    if (*inp->iter != '\0')
                    {
                        char fmt[6];
                        for (msize_t i = 0; i < 6; i++)
                        {
                            fmt[i] = *inp->iter;
                            inp->iter++;
                            if (*inp->iter == '\0')
                            {
                                _READ_ERROR_("Read Error: Syntax Error: Read '.%s' which doesn't represent any attribute. Did you mean 'fmt_bin' or 'fmt_hex'?", fmt);
                                break;
                            }
                        }
                        // CONTINUE FROM HERE
                    }
                    else
                    {
                        // we again got something unexpected
                        _READ_DIRERROR_("Read Error: Syntax Error: Read EOF followed by 'f' which doesn't represent any attribute: Did you mean 'fmt_bin' or 'fmt_hex'?");
                    }
                    break;
                }
                }
            }
            else
            {
                // we will handle this outside
                // but we need an extra error as well
                _READ_DIRERROR_("Read Error: Syntax Error: Expected an attribute after '.' got EOF instead.\n");
            }
        }
        else if (x == ']')
        {
            end_reached = mtrue;
        }
        else
        {
            // we have something unexpected
            _READ_ERROR_("Read Error: Invalid attributes provided: Expected an attribute to start with '.' got '%c' instead.\n", x);
            return RET_FAILURE;
        }
    }
}

MerryInpFile *merry_read_file(mcstr_t _file_name)
{
    // Before opening the file, we need to confirm the extension
    if (merry_check_file_extension(_file_name) == mfalse)
        return RET_NULL; // the file has invalid extension
    // This will read the whole file
    MerryInpFile *inp = (MerryInpFile *)merry_malloc(sizeof(MerryInpFile));
    if (inp == RET_NULL)
    {
        // this was a failure
        _READ_DIRERROR_("Read Internal Error: Couldn't read input file.\n");
        return RET_NULL;
    }
    // we now have to open the file and read it
    inp->_file_name = _file_name;
    inp->f = fopen(_file_name, "r");
    if (inp->f == NULL)
    {
        // something went wrong
        _READ_ERROR_("Read Error: The provided input file name %s is either a directory or doesn't exists.\n", _file_name);
        merry_free(inp);
        return RET_NULL;
    }
    // initialize these to zero
    inp->ilen = 0;
    inp->ibstart = 0;
    inp->ibend = 0;
    inp->dlen = 0;
    inp->dbstart = 0;
    inp->dbend = 0;
    if (merry_read_file_contents(inp) == RET_FAILURE)
    {
        fclose(inp->f);
        merry_free(inp);
        return RET_NULL;
    }
    fclose(inp->f);
}