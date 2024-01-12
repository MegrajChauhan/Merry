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
    if (surelyF(fread(inp->_file_contents, 1, file_len, inp->f) != file_len))
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
    while (((*inp->iter >= 'a') && (*inp->iter <= 'z')) || *inp->iter == '_')
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
        while (((*inp->iter >= '0') && (*inp->iter <= '9')))
        {
            i++;
            inp->iter++;
        }
        // we now have the length of the numbers to read
        char nums[i + 1];
        nums[i] = 0;
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
    while (*inp->iter != '\0' && error_encountered == mfalse && end_reached == mfalse)
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
                char attribute[len + 1];
                attribute[len] = '\0'; // the strncpy function is not terminating attribute with a 0 which is causing these weird characters to be printed beyond attribute's memory
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
                            ibend_provided = mtrue;
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
                    else
                    {
                        _READ_ERROR_("Read Error: The attribute '%s' is not a valid attribute.\n", attribute);
                        error_encountered = mtrue;
                    }
                }
            }
        }
        else if (x == ']')
        {
            end_reached = mtrue;
            inp->iter++; // go beyond
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
        _READ_DIRERROR_("Read Error: Syntax Error: The attribute section was not closed. Expected ']' before EOF.\n");
        return RET_FAILURE;
    }
    // check if we were provided with fmt
    if (fmt_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: File format is not specified.\n");
        return RET_FAILURE;
    }
    if (dlen_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: Data length is not provided.\n");
        return RET_FAILURE;
    }
    if (ilen_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: Instruction length is not provided.\n");
        return RET_FAILURE;
    }
    if (ibstart_provided == mfalse || ibend_provided == mfalse)
    {
        _READ_DIRERROR_("Read Error: Instruction byte start position not clear.\n");
        return RET_FAILURE;
    }
    // the above are a must
    if (inp->ilen == 0)
    {
        // Since there is supposed to be no data bytes, dbstart and dbend are both 0
        _READ_DIRERROR_("Read Error: The ilen attribute provides that there are no instructions which is not allowed.\n");
        return RET_FAILURE;
    }
    if ((inp->ilen % 8) != 0)
    {
        _READ_DIRERROR_("Read Error: The number of instruction bytes is incomplete as indicated by ilen.\n");
        return RET_FAILURE;
    }
    if (inp->dlen != 0)
    {
        // if we have none zero then the data byte positions must be provided
        if (dbstart_provided == mfalse || dbend_provided == mfalse)
        {
            _READ_DIRERROR_("Read Error: Data bytes position not clear.\n");
            return RET_FAILURE;
        }
        // we can check if the positions provided are valid or not as well
        if (inp->dbstart == inp->dbend || inp->dbend < inp->dbstart)
        {
            _READ_DIRERROR_("Read Error: The provided data byte positions are ambigious.\n");
            return RET_FAILURE;
        }
        // check if they overlap with the ibytes
        if ((inp->dbstart >= inp->ibstart && inp->dbend <= inp->ibend) || (inp->dbstart <= inp->ibstart && inp->dbend >= inp->ibend) || (inp->dbstart <= inp->ibstart && (inp->ibstart >= inp->dbend && inp->dbend <= inp->ibend)) || ((inp->dbstart >= inp->ibstart && inp->dbstart <= inp->ibend) && inp->dbend >= inp->ibend))
        {
            _READ_DIRERROR_("Read Error: The instruction bytes and the data bytes overlap.\n");
            return RET_FAILURE;
        }
    }
    else
    {
        // no matter what the file contains
        inp->dbend = 0;
        inp->dbstart = 0;
    }
    // since the overlapping is checked above in case we have data bytes available, we won't have to worry about it here.
    if (inp->ibstart == inp->ibend || inp->ibend < inp->ibstart)
    {
        _READ_DIRERROR_("Read Error: The provided instruction byte positions are ambigious.\n");
        return RET_FAILURE;
    }
    msize_t temp = (inp->ibend + 1) - inp->ibstart;
    // we increment temp here because, there is one more byte than this suggests
    // if ibstart is 0 and ibend is 15, it represents 16 bytes but temp would be 15 so, increment it by 1
    if (((temp) / 8) != (inp->ilen / 8))
    {
        _READ_DIRERROR_("Read Error: The provided instruction length doesn't match with what was observed.\n");
        return RET_FAILURE;
    }
    if ((temp) % 8 != 0)
    {
        _READ_DIRERROR_("Read Error: The provided instruction bytes are not aligned.\n");
        return RET_FAILURE;
    }
    if (inp->ibstart != 0 && inp->dbstart != 0 && ((temp + ((inp->dbend + 1) - inp->dbstart)) != (inp->dlen + inp->ilen)))
    {
        _READ_DIRERROR_("Read Error: It seems neither data nor instruction bytes cover all the bytes.\n");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ mret_t merry_reader_readd_binary(MerryInpFile *inp)
{
    char one_num[65];
    one_num[64] = 0;
    msize_t read_numbers = 0;
    mbool_t error = mfalse;
    msize_t expect = merry_align_size(inp->dlen) / 8;
    while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
    {
        register char x = *inp->iter;
        if (x == ' ' || x == '\n' || x == '\t')
        {
            // skip all the nonsense
            while (x == ' ' || x == '\n' || x == '\t')
            {
                inp->iter++;
                x = *inp->iter;
            }
        }
        if (*inp->iter == '0' || *inp->iter == '1')
        {
            // we have to read the binary numbers now
            msize_t i = 0, j = 0;
            for (i = 0; i < 8; i++)
            {
                if (*inp->iter == '\0')
                    break;
                for (j = 0; j < 8; j++)
                {
                    if (*inp->iter == '\0')
                        break; // we have no more to read
                    if (*inp->iter == '0' || *inp->iter == '1')
                    {
                        one_num[i * 8 + j] = *inp->iter;
                        inp->iter++;
                    }
                    else
                    {
                        _READ_DIRERROR_("Read Error: Expected '1' or '0' but got something unexpected.\n");
                        error = mtrue;
                        break;
                    }
                }
                // any number of space or newlines, tabs can follow this
                x = *inp->iter;
                if (x == ' ' || x == '\n' || x == '\t')
                {
                    while (x == ' ' || x == '\n' || x == '\t')
                    {
                        inp->iter++;
                        x = *inp->iter;
                    }
                }
                if (error == mtrue)
                    break;
            }
            if (error == mfalse)
            {
                // if no error
                // we may have to append 0's at the end
                if (i != 8 || j != 8)
                {
                    // we have to append zeros at the end
                    for (i = i * 8 + j; i < 65; i++)
                    {
                        one_num[i] = '0'; // append with zeros
                    }
                }
                // now convert
                inp->_data[read_numbers] = strtoull(one_num, &one_num, 2);
                printf("READ DATA: %X\n", inp->_data[read_numbers]);
                read_numbers++;
            }
        }
        else
        {
            read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
            error = mtrue;
        }
    }
    return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
}

_MERRY_INTERNAL_ mret_t merry_reader_readd_hexa(MerryInpFile *inp)
{
    char one_num[17];
    one_num[16] = 0;
    msize_t read_numbers = 0;
    mbool_t error = mfalse;
    msize_t expect = merry_align_size(inp->dlen) / 8;
    while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
    {
        register char x = *inp->iter;
        if (x == ' ' || x == '\n' || x == '\t')
        {
            // skip all the nonsense
            while (x == ' ' || x == '\n' || x == '\t')
            {
                inp->iter++;
                x = *inp->iter;
            }
        }
        if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
        {
            // we have to read the binary numbers now
            msize_t i = 0, j = 0;
            for (i = 0; i < 8; i++)
            {
                for (j = 0; j < 2; j++)
                {
                    if (*inp->iter == '\0')
                        break; // we have no more to read
                    if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
                    {
                        one_num[i * 8 + j] = *inp->iter;
                        inp->iter++;
                    }
                    else
                    {
                        _READ_DIRERROR_("Read Error: Expected '0 - 9' or 'a - f' but got something unexpected.\n");
                        error = mtrue;
                        break;
                    }
                }
                // any number of space or newlines, tabs can follow this
                x = *inp->iter;
                if (x == ' ' || x == '\n' || x == '\t')
                {
                    while (x == ' ' || x == '\n' || x == '\t')
                    {
                        inp->iter++;
                        x = *inp->iter;
                    }
                }
                if (error == mtrue)
                    break;
            }
            if (error == mfalse)
            {
                // if no error
                // we may have to append 0's at the end
                if (i != 8 || j != 2)
                {
                    // we have to append zeros at the end
                    for (i = i * 8 + j; i < 17; i++)
                    {
                        one_num[i] = '0'; // append with zeros
                    }
                }
                // now convert
                inp->_data[read_numbers] = strtoull(one_num, &one_num, 16);
                printf("READ DATA: %X\n", inp->_data[read_numbers]);
                read_numbers++;
            }
        }
        else
        {
            read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
            error = mtrue;
        }
    }
    return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
}

/*REDUNDANCY CODE TRIGGER WARNING*/
_MERRY_INTERNAL_ mret_t merry_reader_readi_binary(MerryInpFile *inp)
{
    char one_num[65];
    one_num[64] = 0;
    msize_t read_numbers = 0;
    mbool_t error = mfalse;
    msize_t expect = inp->ilen / 8;
    while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
    {
        register char x = *inp->iter;
        if (x == ' ' || x == '\n' || x == '\t')
        {
            // skip all the nonsense
            while (x == ' ' || x == '\n' || x == '\t')
            {
                inp->iter++;
                x = *inp->iter;
            }
        }
        if (*inp->iter == '0' || *inp->iter == '1')
        {
            // we have to read the binary numbers now
            msize_t i = 0, j = 0;
            for (i = 0; i < 8; i++)
            {
                if (*inp->iter == '\0')
                    break;
                for (j = 0; j < 8; j++)
                {
                    if (*inp->iter == '\0')
                        break; // we have no more to read
                    if (*inp->iter == '0' || *inp->iter == '1')
                    {
                        one_num[i * 8 + j] = *inp->iter;
                        inp->iter++;
                    }
                    else
                    {
                        _READ_DIRERROR_("Read Error: Expected '1' or '0' but got something unexpected.\n");
                        error = mtrue;
                        break;
                    }
                }
                // any number of space or newlines, tabs can follow this
                if (error == mtrue)
                    break;
                x = *inp->iter;
                if (x == ' ' || x == '\n' || x == '\t')
                {
                    while (x == ' ' || x == '\n' || x == '\t')
                    {
                        inp->iter++;
                        x = *inp->iter;
                    }
                }
            }
            if (error == mfalse)
            {
                // if no error
                // convert
                inp->_instructions[read_numbers] = strtoull(one_num, &one_num, 2);
                printf("READ INST: %X\n", inp->_instructions[read_numbers]);
                read_numbers++;
            }
        }
        else
        {
            read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
            error = mtrue;
        }
    }
    return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
}

_MERRY_INTERNAL_ mret_t merry_reader_readi_hexa(MerryInpFile *inp)
{
    char one_num[17];
    one_num[16] = 0;
    msize_t read_numbers = 0;
    mbool_t error = mfalse;
    msize_t expect = (inp->ilen) / 8;
    while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
    {
        register char x = *inp->iter;
        if (x == ' ' || x == '\n' || x == '\t')
        {
            // skip all the nonsense
            while (x == ' ' || x == '\n' || x == '\t')
            {
                inp->iter++;
                x = *inp->iter;
            }
        }
        if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
        {
            // we have to read the binary numbers now
            msize_t i = 0, j = 0;
            for (i = 0; i < 8; i++)
            {
                if (*inp->iter == '\0')
                    break;
                for (j = 0; j < 2; j++)
                {
                    if (*inp->iter == '\0')
                        break; // we have no more to read
                    if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
                    {
                        one_num[i * 8 + j] = *inp->iter;
                        inp->iter++;
                    }
                    else
                    {
                        _READ_DIRERROR_("Read Error: Expected '0 - 9' or 'a - f' but got something unexpected.\n");
                        error = mtrue;
                        break;
                    }
                }
                // any number of space or newlines, tabs can follow this
                if (error == mtrue)
                    break;
                x = *inp->iter;
                if (x == ' ' || x == '\n' || x == '\t')
                {
                    while (x == ' ' || x == '\n' || x == '\t')
                    {
                        inp->iter++;
                        x = *inp->iter;
                    }
                }
            }
            if (error == mfalse)
            {
                // if no error
                inp->_instructions[read_numbers] = strtoull(one_num, &one_num, 16);
                printf("READ INST: %X\n", inp->_instructions[read_numbers]);
                read_numbers++;
            }
        }
        else
        {
            read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
            error = mtrue;
        }
    }
    return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
}

_MERRY_INTERNAL_ mret_t merry_reader_read_data_bytes(MerryInpFile *inp)
{
    // read all of the data bytes
    // we don't have to check if data length is zero or not
    // since this function was called then it means that from the byte that iter is pointing to, the data bytes start
    return inp->_inp_fmt == _FMT_BIN_ ? merry_reader_readd_binary(inp) : merry_reader_readd_hexa(inp);
}

_MERRY_INTERNAL_ mret_t merry_reader_read_inst_bytes(MerryInpFile *inp)
{
    // things like: if ilen is a multiple of 8, the distance between ibstart and ibend is also a multiple of 8 etc are all taken care of
    return inp->_inp_fmt == _FMT_BIN_ ? merry_reader_readi_binary(inp) : merry_reader_readi_hexa(inp);
}

// internal helper: Reads the bytes after reading the attributes
_MERRY_INTERNAL_ mret_t merry_reader_read_bytes(MerryInpFile *inp)
{
    // the iter currently points past the ']' which is the attribute terminator
    // check if we do not have any bytes to read
    // Yes the bytes can follow ']' immediately
    if (*inp->iter == '\0')
    {
        // an error
        _READ_DIRERROR_("Read Error: Exptected to read a byte after attributes but unexpected EOF.\n");
        return RET_FAILURE;
    }
    // we will now read all and every byte
    // since we know how many bytes there are going to be, we can just initialize the arrays just like that
    if (inp->dlen == 0)
    {
        // we have no data
        inp->_data == NULL;
    }
    else
    {
        if (inp->dlen < _MERRY_ALLOC_PAGE_LEN_)
            inp->_data = (mqptr_t)merry_malloc(merry_align_size(inp->dlen));
        else
            inp->_data = (mqptr_t)merry_lalloc(merry_align_size(inp->dlen));
        if (inp->_data == NULL)
            return RET_FAILURE;
    }
    // ilen is a multiple of 8 and we do not care about alignment here
    if (inp->ilen < _MERRY_ALLOC_PAGE_LEN_)
        inp->_instructions = (mqptr_t)merry_malloc(inp->ilen);
    else
        inp->_instructions = (mqptr_t)merry_lalloc(inp->ilen);
    if (inp->_instructions == NULL)
        return RET_FAILURE; // we failed
    if (inp->dlen == 0)
    {
        // everything is the instruction bytes
        if (merry_reader_read_inst_bytes(inp) != RET_SUCCESS)
            return RET_FAILURE;
    }
    else if (inp->dbstart == 0)
    {
        // the data bytes come first
        if (merry_reader_read_data_bytes(inp) != RET_SUCCESS)
            return RET_FAILURE;
        // then the inst bytes
        if (merry_reader_read_inst_bytes(inp) != RET_SUCCESS)
            return RET_FAILURE;
    }
    else
    {
        // the inst bytes come first
        if (merry_reader_read_inst_bytes(inp) != RET_SUCCESS)
            return RET_FAILURE;
        // then data bytes
        if (merry_reader_read_data_bytes(inp) != RET_SUCCESS)
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
    if (merry_reader_read_bytes(inp) != RET_SUCCESS)
    {
        merry_destory_reader(inp);
        return RET_NULL;
    }
    return inp;
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
