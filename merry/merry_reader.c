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
    inp->file_len = file_len;
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
    while (((*inp->iter >= 'a') && (*inp->iter <= 'z')))
    {
        i++;
        inp->iter++;
    }
    return i;
}

_MERRY_INTERNAL_ msize_t merry_reader_read_attr_num(MerryInpFile *inp)
{
    // after reading the attribute, iter must point after the attribute
    // check if we hit the EOF already
    // 0 here indicates error
    if (*inp->iter == '\0')
        return 0; // return error
    // we have to skip all and every space characters
    if (*inp->iter == ' ')
    {
        // until we have no more, we skip
        while (*inp->iter == ' ')
        {
            inp->iter++;
        }
    }
    // after skipping all the whitespaces, check if we stopped before a newline character or reached the end of file
    if (*inp->iter == '\n' || *inp->iter == '\0')
        return 0;
    if (*inp->iter >= '0' && *inp->iter <= '9')
    {
        // we have what we need
        mstr_t temp = inp->iter; // save this for now
        msize_t i = 0;
        while (((*inp->iter >= 'a') && (*inp->iter <= 'z')))
        {
            i++;
            inp->iter++;
        }
        // we now have the length of the numbers to read
        char nums[i];
        if (strncpy(nums, temp, i) != nums)
        {
            read_internal_error();
            return 0;
        }
        else
        {
            // we now have the number in string form
            // since we have read only the valid numbers, this function below will not fail
            return strtoull(nums, &nums, 10);
        }
    }
    else
    {
        // unexpected character
        read_msg("Read Error: Unexpected character '%c' when expected the attribute numbers.", *inp->iter);
        return 0;
    }
    return 0;
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
    mbool_t ibstart_provided = mfalse, ibend_provided = mfalse, dbstart_provided = mfalse, dbend_provided = mfalse;
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
                mstr_t temp = inp->iter; // save the current pos
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
                            // We do not care about if data length is 0
                            inp->dlen = merry_reader_read_attr_num(inp); // get the number
                        }
                    }
                    else if (strcmp(attribute, "ilen") == 0)
                    {
                        // the same as dlen
                        if (ilen_provided == mtrue)
                        {
                            read_double_attr_provided("'instruction length'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            ilen_provided = mtrue;
                            inp->ilen = merry_reader_read_attr_num(inp); // get the number
                        }
                    }
                    else if (strcmp(attribute, "ibstart") == 0)
                    {
                        // after reading the attributes, we will see if the values in ibstart and ibend are valid or not along with dbstart and dbend
                        if (ibstart_provided == mtrue)
                        {
                            read_double_attr_provided("'instruction byte start position'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            ibstart_provided = mtrue;
                            inp->ibstart = merry_reader_read_attr_num(inp);
                            // this can be anything but after the read is complete, the ibstart and ibend must be valid, difference must be a multiple of 8
                        }
                    }
                    else if (strcmp(attribute, "ibend") == 0)
                    {
                        // after reading the attributes, we will see if the values in ibstart and ibend are valid or not along with dbstart and dbend
                        if (ibend_provided == mtrue)
                        {
                            read_double_attr_provided("'instruction byte end position'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            ibstart_provided = mtrue;
                            inp->ibend = merry_reader_read_attr_num(inp);
                        }
                    }
                    else if (strcmp(attribute, "dbstart") == 0)
                    {
                        // after reading the attributes, we will see if the values in ibstart and ibend are valid or not along with dbstart and dbend
                        if (dbstart_provided == mtrue)
                        {
                            read_double_attr_provided("'data byte start position'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            dbstart_provided = mtrue;
                            inp->dbstart = merry_reader_read_attr_num(inp);
                        }
                    }
                    else if (strcmp(attribute, "dbend") == 0)
                    {
                        // after reading the attributes, we will see if the values in ibstart and ibend are valid or not along with dbstart and dbend
                        if (dbend_provided == mtrue)
                        {
                            read_double_attr_provided("'data byte end position'");
                            error_encountered = mtrue;
                        }
                        else
                        {
                            dbend_provided = mtrue;
                            inp->dbend = merry_reader_read_attr_num(inp);
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
    // now that we have parsed all that we could, all that is left is to finalize these results.
    if (error_encountered == mtrue)
    {
        // if we encountered error
        return RET_FAILURE;
    }
    if (end_reached == mfalse)
    {
        _READ_DIRERROR_("Read Error: Syntax Error: The attribute section was not closed. Expected ']' before EOF.");
        return RET_FAILURE;
    }
    // check if we were provided with fmt
    if (fmt_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: File format is not specified.");
        return RET_FAILURE;
    }
    if (dlen_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: Data length is not provided.");
        return RET_FAILURE;
    }
    if (ilen_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: Instruction length is not provided.");
        return RET_FAILURE;
    }
    if (ibstart_provided == mfalse || ibend_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: Instruction byte start position not clear.");
        return RET_FAILURE;
    }
    // the above are a must
    if (inp->ilen == 0)
    {
        // Since there is supposed to be no data bytes, dbstart and dbend are both 0
        _READ_DIRERROR_("Read Error: The ilen attribute provides that there are no instructions which is not allowed.\n");
        return RET_FAILURE;
    }
    else if (inp->ilen % 8 != 0)
    {
        _READ_DIRERROR_("Read Error: The number of instruction bytes is incomplete as indicated by ilen.\n");
        return RET_FAILURE;
    }
    if (inp->dlen != 0)
    {
        // if we have none zero then the data byte positions must be provided
        if (dbstart_provided == mfalse || dbend_provided == mfalse)
        {
            _READ_DIRERROR_("Read Error: Data bytes position not clear.");
            return RET_FAILURE;
        }
        // we can check if the positions provided are valid or not as well
        if (inp->dbstart == inp->dbend || inp->dbend < inp->dbstart)
        {
            _READ_DIRERROR_("Read Error: The provided data byte positions are ambigious.");
            return RET_FAILURE;
        }
        // check if they overlap with the ibytes
        if ((inp->dbstart >= inp->ibstart && inp->dbend <= inp->ibend) || (inp->dbstart <= inp->ibstart && inp->dbend >= inp->ibend) || (inp->dbstart <= inp->ibstart && inp->dbend <= inp->ibend) || (inp->dbstart >= inp->ibstart && inp->dbend >= inp->ibend))
        {
            _READ_DIRERROR_("Read Error: The instruction bytes and the data bytes overlap.");
            return RET_FAILURE;
        }
    }
    // since the overlapping is checked above in case we have data bytes available, we won't have to worry about it here.
    if (inp->ibstart == inp->ibend || inp->ibend < inp->ibstart)
    {
        _READ_DIRERROR_("Read Error: The provided instruction byte positions are ambigious.");
        return RET_FAILURE;
    }
    msize_t temp = inp->ibend - inp->ibstart;
    if ((temp / 8) != (inp->ilen / 8))
    {
        _READ_DIRERROR_("Read Error: The provided instruction length doesn't match with what was observed.");
        return RET_FAILURE;
    }
    if (temp % 8 != 0)
    {
        _READ_DIRERROR_("Read Error: The provided instruction bytes are not aligned.");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
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
        _READ_ERROR_("Read Error: The provided input file name %s is either a directory or doesn't exist.\n", _file_name);
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
    // parse the attributes
    if (merry_reader_parse_attributes(inp) == RET_FAILURE)
    {
        merry_destory_reader(inp);
        return RET_NULL;
    }
    // Now we read the bytes first and convert them into numbers that we can actually use in the VM
    // CONTINUE FROM HERE
    merry_lfree(inp->_file_contents, inp->file_len);
    fclose(inp->f);
}

void merry_destory_reader(MerryInpFile *inp)
{
    if (surelyF(inp == NULL))
        return;
    if (surelyT(inp->f != NULL))
    {
        fclose(inp->f);
    }
    if (surelyT(inp->_data != NULL))
    {
        if (inp->dlen < _MERRY_ALLOC_PAGE_LEN_)
        {
            // most probably allocated
            merry_free(inp->_data);
        }
        else
        {
            merry_lfree(inp->_data, merry_align_size(inp->dlen));
        }
    }
    if (surelyT(inp->_instructions != NULL))
    {
        if (inp->ilen < _MERRY_ALLOC_PAGE_LEN_)
        {
            // most probably allocated
            merry_free(inp->_instructions);
        }
        else
        {
            merry_lfree(inp->_instructions, inp->ilen);
        }
    }
    if (surelyT(inp->_file_contents != NULL))
    {
        merry_lfree(inp->_file_contents, inp->file_len);
    }
    merry_free(inp);
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