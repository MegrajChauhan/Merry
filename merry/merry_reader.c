#include "internals/merry_reader.h"

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
    msize_t aligned = merry_align_size(inp->dlen);
    inp->dpage_count = aligned / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (aligned % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0);
    inp->ipage_count = inp->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (inp->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0);
    // even if dpage_count is 0, we sill need to map one page
    if (inp->dpage_count == 0)
        inp->dpage_count++;
    if (inp->dpage_count < (_MERRY_ALLOC_PAGE_LEN_ / 8))
        inp->_data = (mqptr_t *)merry_malloc(sizeof(mqptr_t *) * inp->dpage_count);
    else
        inp->_data = (mqptr_t *)merry_lalloc(sizeof(mqptr_t *) * inp->dpage_count);
    if (inp->ipage_count < (_MERRY_ALLOC_PAGE_LEN_ / 8))
        inp->_instructions = (mqptr_t *)merry_malloc(sizeof(mqptr_t *) * inp->ipage_count);
    else
        inp->_instructions = (mqptr_t *)merry_lalloc(sizeof(mqptr_t *) * inp->ipage_count);
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
        if (inp->dpage_count < (_MERRY_ALLOC_PAGE_LEN_ / 8))
        {
            // most probably allocated
            merry_free(inp->_data);
        }
        else
        {
            merry_lfree(inp->_data, inp->dpage_count * 8);
        }
    }
    if (surelyT(inp->_instructions != NULL))
    {
        if (inp->ipage_count < (_MERRY_ALLOC_PAGE_LEN_ / 8))
        {
            // most probably allocated
            merry_free(inp->_instructions);
        }
        else
        {
            merry_lfree(inp->_instructions, inp->ipage_count * 8);
        }
    }
    merry_free(inp);
}

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

_MERRY_INTERNAL_ mret_t merry_reader_parse_header(MerryInpFile *inp)
{
    // This will parse the first 24 bytes to collect important information and validate it as well
    mbyte_t header[24]; // 24 bytes
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
    // // the file has the signature bytes
    inp->byte_order = header[7] & 0x1; // get the byte ordering of the input bytes
    // now get the ilen and dlen from SDT
    inp->ilen = ((
                     (
                         (
                             (
                                 (
                                     (
                                         (
                                             (
                                                 (
                                                     (
                                                         (
                                                             (
                                                                 header[8] << 8) &
                                                             header[9])
                                                         << 8) &
                                                     header[10])
                                                 << 8) &
                                             header[11])
                                         << 8) &
                                     header[12])
                                 << 8) &
                             header[13])
                         << 8) &
                     header[14])
                 << 8) &
                header[15];
    // get the data len
    inp->dlen = ((
                     (
                         (
                             (
                                 (
                                     (
                                         (
                                             (
                                                 (
                                                     (
                                                         (
                                                             (header[16] << 8) & header[17])
                                                         << 8) &
                                                     header[18])
                                                 << 8) &
                                             header[19])
                                         << 8) &
                                     header[20])
                                 << 8) &
                             header[21])
                         << 8) &
                     header[22])
                 << 8) &
                header[23];
    // now check if dlen and ilen are within the limits
    if (inp->file_len < (inp->dlen + inp->ilen))
    {
        _READ_DIRERROR_("Read Error: Invalid instruction and data length.\n");
        return RET_FAILURE;
    }
    // we also need to make sure that ilen and dlen are valid
    if (inp->ilen % 8 != 0)
    {
        _READ_DIRERROR_("Read Error: The instruction length is invalid.\n");
        return RET_FAILURE;
    }
    if (inp->dlen % 8 != 0)
    {
        _READ_DIRERROR_("Read Error: The data length is invalid.\n");
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ mret_t merry_reader_byorder_same_i(MerryInpFile *inp)
{
    // read instructions when the bytes ordering is the same as that of the host
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

_MERRY_INTERNAL_ mret_t merry_reader_byorder_diff_i(MerryInpFile *inp)
{
    // read instructions when the ordering of the bytes in the input file is different than that of the host
    // this is not preferable but we can do nothing
    msize_t count = inp->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    msize_t ext = inp->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
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
            inverted[j] = reader_invert_byte_order(num, k);
            k += 8;
        }
        // with the inverted ones, now write them to the mapped memory pages
        if (memcpy(inp->_instructions[i], inverted, _MERRY_MEMORY_ADDRESSES_PER_PAGE_) != inp->_instructions[i])
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
            inverted[j] = reader_invert_byte_order(num, k);
            k += 8;
        }
        // with the inverted ones, now write them to the mapped memory pages
        if (memcpy(inp->_instructions[count], inverted, ext) != inp->_instructions[count])
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
    if (inp->byte_order == _MERRY_BYTE_ORDER_)
    {
        // the ordering is the same as that of the host
        // this makes life a lot easier
        // we can simply read the input bytes one by one to the VM's memory
        ret = merry_reader_byorder_same_i(inp);
    }
    else
        // this indicates the byte ordering is not the same as that of the host, this will be a bit painful
        ret = merry_reader_byorder_diff_i(inp);
    // check if we succeeded
    if (ret == RET_SUCCESS)
        return ret;
    // failed
    // free the mapped pages
    merry_reader_unalloc_pages(inp);
    return ret;
}

_MERRY_INTERNAL_ mret_t merry_reader_byorder_diff_d(MerryInpFile *inp)
{
    // read instructions when the ordering of the bytes in the input file is different than that of the host
    // this is not preferable but we can do nothing
    msize_t count = inp->dlen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    msize_t ext = inp->dlen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
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
            inverted[j] = reader_invert_byte_order(num, k);
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
            inverted[j] = reader_invert_byte_order(num, k);
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
    return RET_SUCCESS; // this should read the bytes properly
}

_MERRY_INTERNAL_ mret_t merry_reader_byorder_same_d(MerryInpFile *inp)
{
    msize_t count = inp->dlen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    msize_t ext = inp->dlen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_; // this gives any remaining addresses
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
    // in future, if we do optimize reader for faster file reading, we can make this function run as a detached thread constantly reading in the background
    return RET_SUCCESS; // we did it!
}

// the same process for data reading as well
_MERRY_INTERNAL_ mret_t merry_reader_read_data(MerryInpFile *inp)
{
    // when it comes to data, even though there may be none, we still map one page
    if (inp->dlen == 0)
        return RET_SUCCESS;
    mret_t ret;
    // the same as instruction reading
    if (inp->byte_order == _MERRY_BYTE_ORDER_)
        ret = merry_reader_byorder_same_d(inp);
    else
        ret = merry_reader_byorder_diff_d(inp);

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
    MerryInpFile *inp = (MerryInpFile *)merry_malloc(sizeof(MerryInpFile));
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
        merry_free(inp);
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