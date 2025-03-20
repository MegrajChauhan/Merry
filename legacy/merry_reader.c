#if defined(_WIN64)
#include "internals\merry_reader.h"
#else
#include "internals/merry_reader.h"
#endif

_MERRY_INTERNAL_ void merry_reader_unalloc_pages(MerryInpFile *inp)
{
    // if we fail while allocating, unmap all
    for (msize_t i = 0; i < inp->dpage_count; i++)
    {
        if ((inp->_data[i]) != _MERRY_RET_GET_ERROR_)
            _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(inp->_data[i]);
    }
    // for instructions
    for (msize_t i = 0; i < inp->ipage_count; i++)
    {
        if (inp->_instructions[i] != _MERRY_RET_GET_ERROR_)
            _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(inp->_instructions[i]);
    }
}

_MERRY_INTERNAL_ mret_t merry_reader_alloc_pages(MerryInpFile *inp)
{
    // after parsing the input file, we need to map the memory pages and prepare for reading
    // In the future, the input files, if gets too large, we have to implement some optimizations for them
    // for example, the reader can read the file in the background and fill the memory as the VM is executing simultaneously
    msize_t aligned = merry_align_size(inp->dlen) + inp->slen; // data includes slen as well
    inp->dpage_count = aligned / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (aligned % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0);
    inp->ipage_count = inp->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (inp->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0);
    // even if dpage_count is 0, we sill need to map one page
    if (inp->dpage_count == 0)
        inp->dpage_count++;
    inp->_data = (mqptr_t *)malloc(sizeof(mqptr_t *) * inp->dpage_count);
    inp->_instructions = (mqptr_t *)malloc(sizeof(mqptr_t *) * inp->ipage_count);
    if (inp->_data == NULL || inp->_instructions == NULL)
        return RET_FAILURE; // we failed
    // now we map the memory for both the instruction page and data page
    for (msize_t i = 0; i < inp->dpage_count; i++)
    {
        if ((inp->_data[i] = _MERRY_MEMORY_PGALLOC_MAP_PAGE_) == _MERRY_RET_GET_ERROR_)
        {
            inp->dpage_count = i;
            merry_reader_unalloc_pages(inp);
            return RET_FAILURE;
        }
    }
    // for instructions
    for (msize_t i = 0; i < inp->ipage_count; i++)
    {
        if ((inp->_instructions[i] = _MERRY_MEMORY_PGALLOC_MAP_PAGE_) == _MERRY_RET_GET_ERROR_)
        {
            inp->ipage_count = i;
            merry_reader_unalloc_pages(inp);
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
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
        // we will not unmap any pages that _data holds on to but we free it
        free(inp->_data);
    }
    if (surelyT(inp->_instructions != NULL))
    {
        free(inp->_instructions);
    }
    free(inp);
}

_MERRY_INTERNAL_ mbool_t merry_check_file_extension(mcstr_t _file_name)
{
    if (surelyF(_file_name == NULL || strlen(_file_name) < 5))
        goto ret_err;
    // now check for the file extension
    mstr_t dot = strrchr(_file_name, '.');
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

_MERRY_INTERNAL_ mret_t merry_reader_parse_header(MerryInpFile *inp)
{
    // This will parse the first 32 bytes to collect important information and validate it as well
    // mbyte_t header[_READER_HEADER_LEN_]; // 32 bytes
    mbyte_t header[_READER_HEADER_LEN_]; // 32 bytes

    // now we read the header
    if (fread(header, 1, _READER_HEADER_LEN_, inp->f) != _READER_HEADER_LEN_)
    {
        // we failed to read the header
        read_internal_error("Unable to read header from the file.");
        return RET_FAILURE;
    }
    // we succeeded but now we have to parse it all up.
    if (header[0] != 0x4d || header[1] != 0x49 || header[2] != 0x4e)
    {
        // we have an error here
        read_msg("Read Error: The input file '%s' doesn't have the signature identifying bytes: Expected \"4d494e\".\n", inp->_file_name);
        return RET_FAILURE;
    }
    // the file has the signature bytes
    // now get the ilen and dlen from SDT

    // we are going to make good use of the remaining 5 bytes of the first 8 bytes
    inp->entry_addr = header[3];
    inp->entry_addr = (inp->entry_addr << 8) | header[4];
    inp->entry_addr = (inp->entry_addr << 8) | header[5];
    inp->entry_addr = (inp->entry_addr << 8) | header[6];
    inp->entry_addr = (inp->entry_addr << 8) | header[7];

    inp->ilen = header[8];
    inp->ilen = (inp->ilen << 8) | header[9];
    inp->ilen = (inp->ilen << 8) | header[10];
    inp->ilen = (inp->ilen << 8) | header[11];
    inp->ilen = (inp->ilen << 8) | header[12];
    inp->ilen = (inp->ilen << 8) | header[13];
    inp->ilen = (inp->ilen << 8) | header[14];
    inp->ilen = (inp->ilen << 8) | header[15];

    inp->dlen = header[16];
    inp->dlen = (inp->dlen << 8) | header[17];
    inp->dlen = (inp->dlen << 8) | header[18];
    inp->dlen = (inp->dlen << 8) | header[19];
    inp->dlen = (inp->dlen << 8) | header[20];
    inp->dlen = (inp->dlen << 8) | header[21];
    inp->dlen = (inp->dlen << 8) | header[22];
    inp->dlen = (inp->dlen << 8) | header[23];

    // get the string len
    inp->slen = header[24];
    inp->slen = (inp->slen << 8) | header[25];
    inp->slen = (inp->slen << 8) | header[26];
    inp->slen = (inp->slen << 8) | header[27];
    inp->slen = (inp->slen << 8) | header[28];
    inp->slen = (inp->slen << 8) | header[29];
    inp->slen = (inp->slen << 8) | header[30];
    inp->slen = (inp->slen << 8) | header[31];

    // now check if dlen and ilen are within the limits
    if (inp->file_len < (inp->dlen + inp->ilen + inp->slen))
    {
        _READ_DIRERROR_("Read Error: Invalid instruction and data length.\n");
        return RET_FAILURE;
    }
    // we also need to make sure that ilen and dlen are valid
    if (inp->ilen % 8 != 0)
    {
        _READ_DIRERROR_("Read Error: The instruction length is invalid. Must be a multiple of 8.\n");
        return RET_FAILURE;
    }
    if ((inp->dlen + inp->slen) % 8 != 0) // string is also data
    {
        _READ_DIRERROR_("Read Error: The data length is invalid. Must be a multiple of 8.\n");
        return RET_FAILURE;
    }
    if (inp->ilen == 0)
    {
        _READ_DIRERROR_("Read Error: The input file has no instructions to read. The input file cannot be empty\n");
        return RET_FAILURE;
    }
    // also check for entry point
    if ((inp->entry_addr * 8) > inp->ilen)
    {
        _READ_DIRERROR_("Read Error: The entry point for the program is outside the bounds of the program's size.\n");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ mret_t merry_reader_read_instructions_same(MerryInpFile *inp)
{
    // we need to know how many pages to read
    // we have ipage_count but we still have to redo the calculations
    msize_t count = inp->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    msize_t ext = inp->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
    for (msize_t i = 0; i < count; i++)
    {
        // now we read
        // we have already made sure that this is within the limits of the input file
        if (fread(inp->_instructions[i], 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, inp->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            // we failed
            // what should we do when we fail to read?
            _READ_DIRERROR_("Read Error: Error while reading instructions.\n");
            return RET_FAILURE;
        }
    }
    // now this will read any remaining ones
    if (ext > 0)
    {
        if (fread(inp->_instructions[count], 1, ext, inp->f) != ext)
        {
            _READ_DIRERROR_("Read Error: Error while reading instructions.\n");
            return RET_FAILURE;
        }
    }
    // in future, if we do optimize reader for faster file reading, we can make this function run as a detached thread constantly reading in the background
    return RET_SUCCESS; // we did it!
}

_MERRY_INTERNAL_ mret_t merry_reader_read_instructions_different(MerryInpFile *inp)
{
    // read instructions when the ordering of the bytes in the input file is different than that of the host
    // this is not preferable but we can do nothing
    register msize_t count = inp->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    register msize_t ext = inp->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
    // how should we approach this? All i can think of is, read 8 bytes, invert them, and write it to the mapped pages
    mbyte_t num[_MERRY_MEMORY_ADDRESSES_PER_PAGE_];
    mqword_t inverted[_MERRY_MEMORY_QS_PER_PAGE_];
    for (msize_t i = 0; i < count; i++)
    {
        if (fread(num, 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, inp->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            _READ_DIRERROR_("Read Error: Error while reading instructions.\n");
            return RET_FAILURE;
        }
        // now we have to invert the order of the bytes
        // now invert all those bytes
        msize_t k = 0;
        for (msize_t j = 0; j < _MERRY_MEMORY_QS_PER_PAGE_; j++)
        {
            inverted[j] = num[k];
            inverted[j] = (inverted[j] << 8) | num[k + 1];
            inverted[j] = (inverted[j] << 8) | num[k + 2];
            inverted[j] = (inverted[j] << 8) | num[k + 3];
            inverted[j] = (inverted[j] << 8) | num[k + 4];
            inverted[j] = (inverted[j] << 8) | num[k + 5];
            inverted[j] = (inverted[j] << 8) | num[k + 6];
            inverted[j] = (inverted[j] << 8) | num[k + 7];
            k += 8;
        }
        // with the inverted ones, now write them to the mapped memory pages
        if (memcpy(inp->_instructions[i], (mbptr_t)inverted, _MERRY_MEMORY_ADDRESSES_PER_PAGE_) != inp->_instructions[i])
        {
            // we failed here
            _READ_DIRERROR_("Read Error: Failed to read instructions.\n");
            return RET_FAILURE;
        }
    }
    if (ext > 0)
    {
        // we have more left
        if (fread(num, 1, ext, inp->f) != ext)
        {
            _READ_DIRERROR_("Read Error: Error while reading instructions.\n");
            return RET_FAILURE;
        }
        msize_t k = 0;
        for (msize_t j = 0; j < (ext / 8); j++)
        {
            inverted[j] = num[k];
            inverted[j] = (inverted[j] << 8) | num[k + 1];
            inverted[j] = (inverted[j] << 8) | num[k + 2];
            inverted[j] = (inverted[j] << 8) | num[k + 3];
            inverted[j] = (inverted[j] << 8) | num[k + 4];
            inverted[j] = (inverted[j] << 8) | num[k + 5];
            inverted[j] = (inverted[j] << 8) | num[k + 6];
            inverted[j] = (inverted[j] << 8) | num[k + 7];
            k += 8;
        }
        // with the inverted ones, now write them to the mapped memory pages
        if (memcpy(inp->_instructions[count], (mbptr_t)inverted, ext) != inp->_instructions[count])
        {
            // we failed here
            _READ_DIRERROR_("Read Error: Failed to read instructions.\n");
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS; // this should read the bytes properly
}

_MERRY_INTERNAL_ mret_t merry_reader_read_inst(MerryInpFile *inp)
{
    // read the instructions which comes first
    // after reading the header along with SDT, we now have all the information needed to read the file
    // first check if the byte ordering of the input bytes is the same as that of the host
    mret_t ret;
    // check if we succeeded
    if (_MERRY_BYTE_ORDER_ == _MERRY_ENDIANNESS_)
        ret = merry_reader_read_instructions_same(inp);
    else
        ret = merry_reader_read_instructions_different(inp);
    if (ret == RET_SUCCESS)
        return ret;
    // failed
    // free the mapped pages
    merry_reader_unalloc_pages(inp);
    return ret;
}

_MERRY_INTERNAL_ mret_t merry_reader_read_data_different(MerryInpFile *inp)
{
    // read instructions when the ordering of the bytes in the input file is different than that of the host
    // this is not preferable but we can do nothing
    register msize_t count = inp->dlen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    register msize_t ext = inp->dlen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
    // how should we approach this? All i can think of is, read 8 bytes, invert them, and write it to the mapped pages
    mbyte_t num[_MERRY_MEMORY_ADDRESSES_PER_PAGE_];
    mqword_t inverted[_MERRY_MEMORY_QS_PER_PAGE_];
    for (msize_t i = 0; i < count; i++)
    {
        if (fread(num, 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, inp->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            _READ_DIRERROR_("Read Error: Error while reading data.\n");
            return RET_FAILURE;
        }
        // now we have to invert the order of the bytes
        // now invert all those bytes
        msize_t k = 0;
        for (msize_t j = 0; j < _MERRY_MEMORY_QS_PER_PAGE_; j++)
        {
            inverted[j] = num[k];
            inverted[j] = (inverted[j] << 8) | num[k + 1];
            inverted[j] = (inverted[j] << 8) | num[k + 2];
            inverted[j] = (inverted[j] << 8) | num[k + 3];
            inverted[j] = (inverted[j] << 8) | num[k + 4];
            inverted[j] = (inverted[j] << 8) | num[k + 5];
            inverted[j] = (inverted[j] << 8) | num[k + 6];
            inverted[j] = (inverted[j] << 8) | num[k + 7];
            k += 8;
        }
        // with the inverted ones, now write them to the mapped memory pages
        if (memcpy(inp->_data[i], inverted, _MERRY_MEMORY_ADDRESSES_PER_PAGE_) != inp->_data[i])
        {
            // we failed here
            _READ_DIRERROR_("Read Error: Failed to read data bytes.\n");
            return RET_FAILURE;
        }
    }
    if (ext > 0)
    {
        // we have more left
        if (fread(num, 1, ext, inp->f) != ext)
        {
            _READ_DIRERROR_("Read Error: Error while reading data.\n");
            return RET_FAILURE;
        }
        msize_t k = 0;
        for (msize_t j = 0; j < (ext / 8); j++)
        {
            inverted[j] = num[k];
            inverted[j] = (inverted[j] << 8) | num[k + 1];
            inverted[j] = (inverted[j] << 8) | num[k + 2];
            inverted[j] = (inverted[j] << 8) | num[k + 3];
            inverted[j] = (inverted[j] << 8) | num[k + 4];
            inverted[j] = (inverted[j] << 8) | num[k + 5];
            inverted[j] = (inverted[j] << 8) | num[k + 6];
            inverted[j] = (inverted[j] << 8) | num[k + 7];
            k += 8;
        }
        // with the inverted ones, now write them to the mapped memory pages
        if (memcpy(inp->_data[count], inverted, ext) != inp->_data[count])
        {
            // we failed here
            _READ_DIRERROR_("Read Error: Failed to read data.\n");
            return RET_FAILURE;
        }
    }

    // we will read string here
    if (inp->slen == 0)
        return RET_SUCCESS;

    inp->dlen += inp->slen;

    // firstly read
    mqword_t rem = _MERRY_MEMORY_ADDRESSES_PER_PAGE_ - ext;

    if (fread(inp->_data[count] + (ext / 8), 1, rem < inp->slen ? rem : inp->slen, inp->f) != (rem < inp->slen ? rem : inp->slen))
    {
        _READ_DIRERROR_("Read Error: Failed to read string.\n");
        return RET_FAILURE;
    }

    if (inp->slen < rem)
        return RET_SUCCESS; // we have read what we can

    // we have read another page now
    inp->slen -= rem; // we have read rem number of bytes already

    register msize_t s_count = inp->slen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    register msize_t s_ext = inp->slen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    count++;
    for (msize_t i = 0; i < (s_count); i++)
    {
        if (fread(inp->_data[count], 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, inp->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            // we failed
            // what should we do when we fail to read?
            _READ_DIRERROR_("Read Error: Failed to read string data.\n");
            return RET_FAILURE;
        }
        count++;
    }

    if (s_ext > 0)
    {
        if (fread(inp->_data[count], 1, s_ext, inp->f) != s_ext)
        {
            _READ_DIRERROR_("Read Error: Failed to read string data.\n");
            return RET_FAILURE;
        }
    }

    return RET_SUCCESS; // this should read the bytes properly
}

_MERRY_INTERNAL_ mret_t merry_reader_read_data_same(MerryInpFile *inp)
{
    register msize_t count = inp->dlen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    register msize_t ext = inp->dlen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
    for (msize_t i = 0; i < count; i++)
    {
        // now we read
        // we have already made sure that this is within the limits of the input file
        if (fread(inp->_data[i], 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, inp->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            // we failed
            // what should we do when we fail to read?
            _READ_DIRERROR_("Read Error: Failed to read data.\n");
            return RET_FAILURE;
        }
    }
    // now this will read any remaining ones
    if (ext > 0)
    {
        if (fread(inp->_data[count], 1, ext, inp->f) != ext)
        {
            _READ_DIRERROR_("Read Error: Failed to read data.\n");
            return RET_FAILURE;
        }
    }
    // we will read string here
    if (inp->slen == 0)
        return RET_SUCCESS;

    // firstly read
    mqword_t rem = _MERRY_MEMORY_ADDRESSES_PER_PAGE_ - ext;

    if (fread(inp->_data[count] + (ext / 8), 1, rem < inp->slen ? rem : inp->slen, inp->f) != (rem < inp->slen ? rem : inp->slen))
    {
        _READ_DIRERROR_("Read Error: Failed to read string.\n");
        return RET_FAILURE;
    }
    if (inp->slen < rem)
        return RET_SUCCESS; // we have read what we can

    inp->dlen += inp->slen;

    // we have read another page now
    inp->slen -= rem; // we have read rem number of bytes already

    register msize_t s_count = inp->slen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    register msize_t s_ext = inp->slen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    count++;
    for (msize_t i = 0; i < (s_count); i++)
    {
        if (fread(inp->_data[count], 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, inp->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            // we failed
            // what should we do when we fail to read?
            _READ_DIRERROR_("Read Error: Failed to read string data.\n");
            return RET_FAILURE;
        }
        count++;
    }

    if (s_ext > 0)
    {
        if (fread(inp->_data[count], 1, s_ext, inp->f) != s_ext)
        {
            _READ_DIRERROR_("Read Error: Failed to read string data.\n");
            return RET_FAILURE;
        }
    }

    // in future, if we do optimize reader for faster file reading, we can make this function run as a detached thread constantly reading in the background
    return RET_SUCCESS; // we did it!
}

// the same process for data reading as well
_MERRY_INTERNAL_ mret_t merry_reader_read_data(MerryInpFile *inp)
{
    // when it comes to data, even though there may be none, we still map one page
    if ((inp->dlen + inp->slen) == 0)
        return RET_SUCCESS;
    mret_t ret;
    if (_MERRY_BYTE_ORDER_ == _MERRY_ENDIANNESS_)
        ret = merry_reader_read_data_same(inp);
    else
        ret = merry_reader_read_data_different(inp);
    // the same as instruction reading
    if (ret == RET_SUCCESS)
        return ret;
    merry_reader_unalloc_pages(inp);
    return ret;
}

MerryInpFile *merry_read_file(mcstr_t _file_name)
{
    // read the input file
    // this is the updated reader
    // unlike the previous reader, we will use bytes to perform reading
    // check if the file has proper extension
    if (merry_check_file_extension(_file_name) == mfalse)
        return RET_NULL; // the file has invalid extension
    MerryInpFile *inp = (MerryInpFile *)malloc(sizeof(MerryInpFile));
    if (inp == RET_NULL)
    {
        // this was a failure
        _READ_DIRERROR_("Read Internal Error: Couldn't read input file.\n");
        return RET_NULL;
    }
    // we now have to open the file and read it
    inp->_file_name = _file_name;
    // open the file for reading
    inp->f = fopen(_file_name, "rb");
    if (inp->f == NULL)
    {
        // something went wrong
        _READ_ERROR_("Read Error: The provided input file name %s is either a directory or doesn't exist.\n", _file_name);
        free(inp);
        return RET_NULL;
    }
    // get the length of the file
    inp->file_len = merry_get_file_len(inp->f);
    if (inp->file_len == 0 || inp->file_len < 24)
    {
        _READ_ERROR_("Read Error: The file '%s' is empty.\n", _file_name);
        goto failed;
    }
    // read the header
    if (merry_reader_parse_header(inp) != RET_SUCCESS)
        goto failed;
    // now that we have validated the header, we can read the header
    // but first map necessary pages
    if (merry_reader_alloc_pages(inp) != RET_SUCCESS)
        goto failed;
    // now we can read
    if (merry_reader_read_inst(inp) != RET_SUCCESS || merry_reader_read_data(inp) != RET_SUCCESS)
        goto failed;
    return inp;
failed:
    merry_destory_reader(inp);
    return RET_NULL;
}