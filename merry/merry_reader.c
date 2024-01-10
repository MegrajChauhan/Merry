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

_MERRY_INTERNAL_ msize_t merry_reader_get_attribute_len(MerryInpFile *inp)
{
    // from the current iterator's position, get an attribute
    msize_t i = 0;
    while (((*inp->iter >= 'a') && (*inp->iter <= 'z')) && *inp->iter != '\0')
    {
        i++;
        inp->iter++;
    }
    return i;
}

_MERRY_INTERNAL_ msize_t merry_reader_read_attr_num(MerryInpFile *inp)
{
    // after reading the attribute, iter must pointer after the attribute
    // check if we hit the EOF already
    // 0 here indicates error
    msize_t ret = 0;
    if (*inp->iter == '\0')
        return 0; // return error
    // we have to skip all and every space characters
    if (*inp->iter == ' ')
    {
        // until we have no more, we skip
        while (*inp->iter == ' ' && *inp->iter != '\0')
        {
            inp->iter++;
        }
        // after skipping all the whitespaces, check if we stopped before a newline character or reached the end of file
        if (*inp->iter == '\n' || *inp->iter == '\0')
            return 0;
        if (*inp->iter >= '0' && *inp->iter <= '9')
        {
            // we have what we need
        }
        else
        {
            // unexpected character
            // read_msg("Unexpected character when expected the attribute numbers");
            // CONTINUE FROM HERE
        }
    }
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
    mbool_t error_encountered = mfalse;
    mbool_t end_reached = mfalse;
    while (*inp->iter != ']' && *inp->iter != '\0' && error_encountered == mfalse)
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
            if (*inp->iter == '\0')
            {
                // we will handle this outside
                // but we need an extra error as well
                read_unexpected_eof();
                error_encountered = mtrue;
            }
            else
            {
                // we shall get what the attribute is
                mstr_t temp = inp->iter; // save the current state
                msize_t len = merry_reader_get_attribute_len(inp);
                char attribute[len];
                if (strncpy(attribute, temp, len) != attribute)
                {
                    read_internal_error();
                    error_encountered = mtrue;
                }
                else
                {

                    // we we just compare attribute with lots of things
                    if (strcmp(attribute, "fmt_bin") == 0)
                    {
                        // this file is supposed to be in binary file format
                        if (fmt_provided == mtrue)
                        {
                            // we were already provided with this attribute
                            read_double_attr_provided("'format'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            // this is the first time we got this attribute
                            fmt_provided = mtrue;
                            inp->_inp_fmt = _FMT_BIN_;
                        }
                    }
                    else if (strcmp(attribute, "fmt_hex") == 0)
                    {
                        // this file is supposed to be in hex file format
                        if (fmt_provided == mtrue)
                        {
                            // we were already provided with this attribute
                            read_double_attr_provided("'format'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            // this is the first time we got this attribute
                            fmt_provided = mtrue;
                            inp->_inp_fmt = _FMT_HEX_;
                        }
                    }
                    else if (strcmp(attribute, "dlen") == 0)
                    {
                        // the data len attribute is being provided
                        // We are expecting the number to be the number of bytes which will be aligned by the reader
                        if (dlen_provided == mtrue)
                        {
                            read_double_attr_provided("'data length'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            dlen_provided = mtrue;
                            // now we need to read the numbers after this
                            // since we just read the attribute, we expect a space or any arbitary number of spaces followed by the number
                            // but if we have a newline before we read the number then it is an error
                            // iter points to the new
                        }
                    }
                }
            }
        }
        else if (x == ']')
        {
            end_reached = mtrue;
        }
        else
        {
            // we have something unexpected
            _READ_ERROR_("Read Error: Invalid attributes provided: Expected an attribute that starts with '.' got '%c' instead.\n", x);
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

// switch (*inp->iter)
// {
// case 'f':
// {
//     // the file format is being specified
//     if (fmt_provided == mtrue)
//     {
//         read_double_attr_provided("format");
//         error_encountered = mtrue;
//         break; // we are done
//     }
//     inp->iter++;

//     if (*inp->iter == '\0')
//     {
//         // we again got something unexpected
//         read_unexpected_eof_when("fmt_bin or fmt_hex");
//         error_encountered = mtrue;
//     }
//     else
//     {
//         char fmt[6];
//         for (msize_t i = 0; i < 6; i++)
//         {
//             fmt[i] = *inp->iter;
//             inp->iter++;
//             if (*inp->iter == '\0')
//             {
//                 read_unexpected_eof_when("fmt_bin or fmt_hex");
//                 error_encountered = mtrue;
//                 break;
//             }
//         }
//         if (strcmp(fmt, "mt_bin") == 0)
//         {
//             fmt_provided = mtrue;
//             inp->_inp_fmt = _FMT_BIN_;
//         }
//         else if (strcmp(fmt, "mt_hex") == 0)
//         {
//             fmt_provided = mtrue;
//             inp->_inp_fmt = _FMT_HEX_;
//         }
//         else
//         {
//             read_expected("'fmt_bin' or 'fmt_hex'");
//             error_encountered = mtrue;
//         }
//     }
//     break;
// }
// case 'i':
// {
//     // this is most probably related to instruction related attribute
//     inp->iter++;
//     if (*inp->iter == '\0')
//     {
//         read_unexpected_eof();
//         error_encountered = mtrue;
//     }
//     else
//     {
//         if (*inp->iter == 'l')
//         {
//             // this is providing the ilen attribute
//             // we expect the ilen attribute here followed by any number of spaces and a number before a newline
//         }
//         else if (*inp->iter == 'b')
//         {
//             // we are getting the bytes attribute
//         }
//     }
// }
// }