#include "internals/merry_reader.h"

// _MERRY_INTERNAL_ mret_t merry_read_file_contents(MerryInpFile *inp)
// {
//     // get the length of the input file
//     msize_t file_len = merry_get_file_len(inp->f);
//     if (file_len == 0)
//     {
//         _READ_DIRERROR_("Read Error: The input file is empty. Expected to start with '['.\n");
//         return RET_FAILURE;
//     }
//     inp->file_len = file_len;
//     inp->_file_contents = (mstr_t)merry_lalloc(file_len);
//     if (inp->_file_contents == RET_NULL)
//     {
//         _READ_DIRERROR_("Read Internal Error: Failed to read the input file.\n");
//         return RET_FAILURE;
//     }
//     if (surelyF(fread(inp->_file_contents, 1, file_len, inp->f) != file_len))
//     {
//         merry_lfree(inp->_file_contents, file_len);
//         _READ_DIRERROR_("Read Internal Error: Failed to read the input file.\n");
//         return RET_FAILURE;
//     }
//     inp->iter = inp->_file_contents;
//     return RET_SUCCESS; // we read the file successfully
// }

// _MERRY_INTERNAL_ msize_t merry_reader_get_attribute_len(MerryInpFile *inp)
// {
//     // from the current iterator's position, get an attribute
//     msize_t i = 0;
//     while (((*inp->iter >= 'a') && (*inp->iter <= 'z')) || *inp->iter == '_')
//     {
//         i++;
//         inp->iter++;
//     }
//     return i;
// }

// _MERRY_INTERNAL_ msize_t merry_reader_read_attr_num(MerryInpFile *inp)
// {
//     // after reading the attribute, iter must point after the attribute
//     // check if we hit the EOF already
//     // 0 here indicates error
//     if (*inp->iter == '\0')
//         return 0; // return error
//     // we have to skip all and every space characters
//     if (*inp->iter == ' ')
//     {
//         // until we have no more, we skip
//         while (*inp->iter == ' ')
//         {
//             inp->iter++;
//         }
//     }
//     // after skipping all the whitespaces, check if we stopped before a newline character or reached the end of file
//     if (*inp->iter == '\n' || *inp->iter == '\0')
//         return 0;
//     if (*inp->iter >= '0' && *inp->iter <= '9')
//     {
//         // we have what we need
//         mstr_t temp = inp->iter; // save this for now
//         msize_t i = 0;
//         while (((*inp->iter >= '0') && (*inp->iter <= '9')))
//         {
//             i++;
//             inp->iter++;
//         }
//         // we now have the length of the numbers to read
//         char nums[i + 1];
//         nums[i] = 0;
//         if (strncpy(nums, temp, i) != nums)
//         {
//             read_internal_error();
//             return 0;
//         }
//         else
//         {
//             // we now have the number in string form
//             // since we have read only the valid numbers, this function below will not fail
//             return strtoull(nums, &nums, 10);
//         }
//     }
//     else
//     {
//         // unexpected character
//         read_msg("Read Error: Unexpected character '%c' when expected the attribute numbers.", *inp->iter);
//         return 0;
//     }
//     return 0;
// }

// _MERRY_INTERNAL_ void merry_reader_unalloc_pages(MerryInpFile *inp)
// {
//     // if we fail while allocating, unmap all
//     for (msize_t i = 0; i < inp->dpage_count; i++)
//     {
//         if ((inp->_data[i]) != _MERRY_RET_GET_ERROR_)
//             _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(inp->_data[i]);
//     }
//     // for instructions
//     for (msize_t i = 0; i < inp->ipage_count; i++)
//     {
//         if (inp->_instructions[i] != _MERRY_RET_GET_ERROR_)
//             _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(inp->_instructions[i]);
//     }
// }

// _MERRY_INTERNAL_ mret_t merry_reader_alloc_pages(MerryInpFile *inp)
// {
//     // after parsing the input file, we need to map the memory pages and prepare for reading
//     // In the future, the input files, if get too large, we have to implement some optimizations for them
//     // for example, the reader can read the file in the background and fill the memory as the VM is executing simultaneously
//     msize_t aligned = merry_align_size(inp->dlen);
//     inp->dpage_count = aligned / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (aligned % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0);
//     inp->ipage_count = inp->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (inp->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0);
//     // even if dpage_count is 0, we sill need to map one page
//     if (inp->dpage_count == 0)
//         inp->dpage_count++;
//     if (inp->dpage_count < (_MERRY_ALLOC_PAGE_LEN_ / 8))
//         inp->_data = (mqptr_t *)merry_malloc(sizeof(mqptr_t *) * inp->dpage_count);
//     else
//         inp->_data = (mqptr_t *)merry_lalloc(sizeof(mqptr_t *) * inp->dpage_count);
//     if (inp->ipage_count < (_MERRY_ALLOC_PAGE_LEN_ / 8))
//         inp->_instructions = (mqptr_t *)merry_malloc(sizeof(mqptr_t *) * inp->ipage_count);
//     else
//         inp->_instructions = (mqptr_t *)merry_lalloc(sizeof(mqptr_t *) * inp->ipage_count);
//     if (inp->_data == NULL || inp->_instructions == NULL)
//         return RET_FAILURE; // we failed
//     // now we map the memory for both the instruction page and data page
//     for (msize_t i = 0; i < inp->dpage_count; i++)
//     {
//         if ((inp->_data[i] = _MERRY_MEMORY_PGALLOC_MAP_PAGE_) == _MERRY_RET_GET_ERROR_)
//         {
//             inp->dpage_count = i;
//             merry_reader_unalloc_pages(inp);
//             return RET_FAILURE;
//         }
//     }
//     // for instructions
//     for (msize_t i = 0; i < inp->ipage_count; i++)
//     {
//         {
//             inp->ipage_count = i;
//             merry_reader_unalloc_pages(inp);
//             return RET_FAILURE;
//         }
//     }
//     return RET_SUCCESS;
// }

// _MERRY_INTERNAL_ mret_t merry_reader_readd_binary(MerryInpFile *inp)
// {
//     char one_num[65];
//     one_num[64] = 0;
//     mstr_t ptr = &one_num;
//     msize_t read_numbers = 0;
//     mbool_t error = mfalse;
//     msize_t expect = merry_align_size(inp->dlen) / 8;
//     msize_t current_page = 0;
//     while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
//     {
//         ptr = &one_num;
//         register char x = *inp->iter;
//         if (x == ' ' || x == '\n' || x == '\t')
//         {
//             // skip all the nonsense
//             while (x == ' ' || x == '\n' || x == '\t')
//             {
//                 inp->iter++;
//                 x = *inp->iter;
//             }
//         }
//         if (*inp->iter == '0' || *inp->iter == '1')
//         {
//             // we have to read the binary numbers now
//             msize_t i = 0, j = 0;
//             for (i = 0; i < 8; i++)
//             {
//                 if (*inp->iter == '\0')
//                     break;
//                 for (j = 0; j < 8; j++)
//                 {
//                     if (*inp->iter == '\0')
//                         break; // we have no more to read
//                     if (*inp->iter == '0' || *inp->iter == '1')
//                     {
//                         *ptr = *inp->iter;
//                         ptr++;
//                         inp->iter++;
//                     }
//                     else
//                     {
//                         _READ_DIRERROR_("Read Error: Expected '1' or '0' but got something unexpected.\n");
//                         error = mtrue;
//                         break;
//                     }
//                 }
//                 // any number of space or newlines, tabs can follow this
//                 x = *inp->iter;
//                 if (x == ' ' || x == '\n' || x == '\t')
//                 {
//                     while (x == ' ' || x == '\n' || x == '\t')
//                     {
//                         inp->iter++;
//                         x = *inp->iter;
//                     }
//                 }
//                 if (error == mtrue)
//                     break;
//             }
//             if (error == mfalse)
//             {
//                 // if no error
//                 // we may have to append 0's at the end
//                 if (ptr != &one_num[64])
//                 {
//                     // we have to append zeros at the end
//                     for (; ptr != &one_num[64]; ptr++)
//                     {
//                         *ptr = '0'; // append with zeros
//                     }
//                 }
//                 // now convert
//                 inp->_data[current_page][read_numbers] = strtoull(one_num, &one_num, 2);
//                 read_numbers++;
//                 if (read_numbers == _MERRY_MEMORY_QS_PER_PAGE_)
//                 {
//                     read_numbers = 0;
//                     current_page++;
//                 }
//             }
//         }
//         else
//         {
//             read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
//             error = mtrue;
//         }
//     }
//     return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
// }

// _MERRY_INTERNAL_ mret_t merry_reader_readd_hexa(MerryInpFile *inp)
// {
//     char one_num[17];
//     mstr_t ptr = &one_num;
//     one_num[16] = 0;
//     msize_t read_numbers = 0;
//     mbool_t error = mfalse;
//     msize_t expect = merry_align_size(inp->dlen) / 8;
//     msize_t current_page = 0;
//     while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
//     {
//         ptr = &one_num;
//         register char x = *inp->iter;
//         if (x == ' ' || x == '\n' || x == '\t')
//         {
//             // skip all the nonsense
//             while (x == ' ' || x == '\n' || x == '\t')
//             {
//                 inp->iter++;
//                 x = *inp->iter;
//             }
//         }
//         if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
//         {
//             // we have to read the binary numbers now
//             msize_t i = 0, j = 0;
//             for (i = 0; i < 8; i++)
//             {
//                 if (*inp->iter == '\0')
//                     break;
//                 for (j = 0; j < 2; j++)
//                 {
//                     if (*inp->iter == '\0')
//                         break;
//                     if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
//                     {
//                         *ptr = *inp->iter;
//                         ptr++;
//                         inp->iter++;
//                     }
//                     else
//                     {
//                         _READ_DIRERROR_("Read Error: Expected '0 - 9' or 'a - f' but got something unexpected.\n");
//                         error = mtrue;
//                         break;
//                     }
//                 }
//                 // any number of space or newlines, tabs can follow this
//                 x = *inp->iter;
//                 if (x == ' ' || x == '\n' || x == '\t')
//                 {
//                     while (x == ' ' || x == '\n' || x == '\t')
//                     {
//                         inp->iter++;
//                         x = *inp->iter;
//                     }
//                 }
//                 if (error == mtrue)
//                     break;
//             }
//             if (error == mfalse)
//             {
//                 // if no error
//                 // we may have to append 0's at the end
//                 if (ptr != &one_num[16])
//                 {
//                     // we have to append zeros at the end
//                     for (; ptr != &one_num[16]; ptr++)
//                     {
//                         *ptr = '0'; // append with zeros
//                     }
//                 }
//                 // now convert
//                 inp->_data[current_page][read_numbers] = strtoull(one_num, &one_num, 2);
//                 read_numbers++;
//                 if (read_numbers == _MERRY_MEMORY_QS_PER_PAGE_)
//                 {
//                     read_numbers = 0;
//                     current_page++;
//                 }
//             }
//         }
//         else
//         {
//             read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
//             error = mtrue;
//         }
//     }
//     return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
// }

// /*REDUNDANCY CODE TRIGGER WARNING*/
// _MERRY_INTERNAL_ mret_t merry_reader_readi_binary(MerryInpFile *inp)
// {
//     char one_num[65];
//     one_num[64] = 0;
//     msize_t read_numbers = 0;
//     mbool_t error = mfalse;
//     msize_t expect = inp->ilen / 8;
//     msize_t current_page = 0;
//     while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
//     {
//         register char x = *inp->iter;
//         if (x == ' ' || x == '\n' || x == '\t')
//         {
//             // skip all the nonsense
//             while (x == ' ' || x == '\n' || x == '\t')
//             {
//                 inp->iter++;
//                 x = *inp->iter;
//             }
//         }
//         if (*inp->iter == '0' || *inp->iter == '1')
//         {
//             // we have to read the binary numbers now
//             msize_t i = 0, j = 0;
//             for (i = 0; i < 8; i++)
//             {
//                 if (*inp->iter == '\0')
//                     break;
//                 for (j = 0; j < 8; j++)
//                 {
//                     if (*inp->iter == '\0')
//                         break; // we have no more to read
//                     if (*inp->iter == '0' || *inp->iter == '1')
//                     {
//                         one_num[i * 8 + j] = *inp->iter;
//                         inp->iter++;
//                     }
//                     else
//                     {
//                         _READ_DIRERROR_("Read Error: Expected '1' or '0' but got something unexpected.\n");
//                         error = mtrue;
//                         break;
//                     }
//                 }
//                 // any number of space or newlines, tabs can follow this
//                 if (error == mtrue)
//                     break;
//                 x = *inp->iter;
//                 if (x == ' ' || x == '\n' || x == '\t')
//                 {
//                     while (x == ' ' || x == '\n' || x == '\t')
//                     {
//                         inp->iter++;
//                         x = *inp->iter;
//                     }
//                 }
//             }
//             if (error == mfalse)
//             {
//                 // if no error
//                 // convert
//                 inp->_instructions[current_page][read_numbers] = strtoull(one_num, &one_num, 2);
//                 read_numbers++;
//                 if (read_numbers == _MERRY_MEMORY_QS_PER_PAGE_)
//                 {
//                     read_numbers = 0;
//                     current_page++;
//                 }
//             }
//         }
//         else
//         {
//             read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
//             error = mtrue;
//         }
//     }
//     return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
// }

// _MERRY_INTERNAL_ mret_t merry_reader_readi_hexa(MerryInpFile *inp)
// {
//     char one_num[17];
//     one_num[16] = 0;
//     msize_t read_numbers = 0;
//     mbool_t error = mfalse;
//     msize_t expect = (inp->ilen) / 8;
//     msize_t current_page = 0;
//     while (*inp->iter != '\0' && error == mfalse && read_numbers != expect)
//     {
//         register char x = *inp->iter;
//         if (x == ' ' || x == '\n' || x == '\t')
//         {
//             // skip all the nonsense
//             while (x == ' ' || x == '\n' || x == '\t')
//             {
//                 inp->iter++;
//                 x = *inp->iter;
//             }
//         }
//         if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
//         {
//             // we have to read the binary numbers now
//             msize_t i = 0, j = 0;
//             for (i = 0; i < 8; i++)
//             {
//                 if (*inp->iter == '\0')
//                     break;
//                 for (j = 0; j < 2; j++)
//                 {
//                     if (*inp->iter == '\0')
//                         break; // we have no more to read
//                     if ((*inp->iter >= '0' && *inp->iter <= '9') || (*inp->iter >= 'a' && *inp->iter <= 'f'))
//                     {
//                         one_num[i * 2 + j] = *inp->iter;
//                         inp->iter++;
//                     }
//                     else
//                     {
//                         _READ_DIRERROR_("Read Error: Expected '0 - 9' or 'a - f' but got something unexpected.\n");
//                         error = mtrue;
//                         break;
//                     }
//                 }
//                 // any number of space or newlines, tabs can follow this
//                 if (error == mtrue)
//                     break;
//                 x = *inp->iter;
//                 if (x == ' ' || x == '\n' || x == '\t')
//                 {
//                     while (x == ' ' || x == '\n' || x == '\t')
//                     {
//                         inp->iter++;
//                         x = *inp->iter;
//                     }
//                 }
//             }
//             if (error == mfalse)
//             {
//                 // if no error
//                 inp->_instructions[current_page][read_numbers] = strtoull(one_num, &one_num, 2);
//                 read_numbers++;
//                 if (read_numbers == _MERRY_MEMORY_QS_PER_PAGE_)
//                 {
//                     read_numbers = 0;
//                     current_page++;
//                 }
//             }
//         }
//         else
//         {
//             read_msg("Read Error: Expected '1' or '0' but got '%c' instead.\n", *inp->iter);
//             error = mtrue;
//         }
//     }
//     return error == mtrue || expect != read_numbers ? RET_FAILURE : RET_SUCCESS;
// }

// _MERRY_INTERNAL_ mret_t merry_reader_read_data_bytes(MerryInpFile *inp)
// {
//     // read all of the data bytes
//     // we don't have to check if data length is zero or not
//     // since this function was called then it means that from the byte that iter is pointing to, the data bytes start
//     return inp->_inp_fmt == _FMT_BIN_ ? merry_reader_readd_binary(inp) : merry_reader_readd_hexa(inp);
// }

// _MERRY_INTERNAL_ mret_t merry_reader_read_inst_bytes(MerryInpFile *inp)
// {
//     // things like: if ilen is a multiple of 8, the distance between ibstart and ibend is also a multiple of 8 etc are all taken care of
//     return inp->_inp_fmt == _FMT_BIN_ ? merry_reader_readi_binary(inp) : merry_reader_readi_hexa(inp);
// }

// _MERRY_INTERNAL_ mret_t merry_reader_read_bytes(MerryInpFile *inp)
// {
//     // the iter currently points past the ']' which is the attribute terminator
//     // check if we do not have any bytes to read
//     // Yes the bytes can follow ']' immediately
//     if (*inp->iter == '\0')
//     {
//         // an error
//         _READ_DIRERROR_("Read Error: Exptected to read a byte after attributes but unexpected EOF.\n");
//         return RET_FAILURE;
//     }
//     // we will now read all and every byte
//     if (inp->_sec_start == _SEC_START_DATA_)
//     {
//         // the data bytes come first
//         if (merry_reader_read_data_bytes(inp) != RET_SUCCESS)
//             return RET_FAILURE;
//         // then the inst bytes
//         if (merry_reader_read_inst_bytes(inp) != RET_SUCCESS)
//             return RET_FAILURE;
//     }
//     else
//     {
//         // the inst bytes come first
//         if (merry_reader_read_inst_bytes(inp) != RET_SUCCESS)
//             return RET_FAILURE;
//         // then data bytes
//         if (merry_reader_read_data_bytes(inp) != RET_SUCCESS)
//             return RET_FAILURE;
//     }
//     return RET_SUCCESS;
// }

// MerryInpFile *merry_read_file(mcstr_t _file_name)
// {
//     // Before opening the file, we need to confirm the extension
//     if (merry_check_file_extension(_file_name) == mfalse)
//         return RET_NULL; // the file has invalid extension
//     // This will read the whole file
//     MerryInpFile *inp = (MerryInpFile *)merry_malloc(sizeof(MerryInpFile));
//     if (inp == RET_NULL)
//     {
//         // this was a failure
//         _READ_DIRERROR_("Read Internal Error: Couldn't read input file.\n");
//         return RET_NULL;
//     }
//     // we now have to open the file and read it
//     inp->_file_name = _file_name;
//     inp->f = fopen(_file_name, "r");
//     if (inp->f == NULL)
//     {
//         // something went wrong
//         _READ_ERROR_("Read Error: The provided input file name %s is either a directory or doesn't exist.\n", _file_name);
//         merry_free(inp);
//         return RET_NULL;
//     }
//     // initialize these to zero
//     inp->ilen = 0;
//     inp->dlen = 0;
//     if (merry_read_file_contents(inp) == RET_FAILURE)
//     {
//         fclose(inp->f);
//         merry_free(inp);
//         return RET_NULL;
//     }
//     // parse the attributes
//     if (merry_reader_parse_attributes(inp) == RET_FAILURE)
//         goto failure;
//     // Now we read the bytes first and convert them into numbers that we can actually use in the VM
//     if (merry_reader_alloc_pages(inp) != RET_SUCCESS)
//         goto failure;
//     // if (merry_reader_read_bytes(inp) != RET_SUCCESS)
//     // {
//     //     merry_destory_reader(inp);
//     //     return RET_NULL;
//     // }
//     if (*inp->iter != '\0')
//     {
//         // if we have not reached the end yet
//         _READ_DIRERROR_("Read Error: Expected EOF at the end but it seems like we have more bytes.\n");
//         goto failure;
//     }
//     return inp;
// failure:
//     merry_destory_reader(inp); // we failed
//     return RET_NULL;
// }

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
    // This will parse the first 8 bytes to collect important information and validate it as well
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
        read_msg("Read Error: The input file '%s' doesn't have the signature identifying bytes.\n", inp->_file_name);
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
            // simply indicate failure for now
            return RET_FAILURE;
        }
    }
    // now this will read any remaining ones
    if (ext > 0)
    {
        if (fread(inp->_instructions[count], 1, ext, inp->f) != ext)
            return RET_FAILURE;
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
    // how should we approach
}

_MERRY_INTERNAL_ mret_t merry_reader_read_inst(MerryInpFile *inp)
{
    // read the instructions which comes first
    // after reading the header along with SDT, we now have all the information needed to read the file
    // first check if the byte ordering of the input bytes is the same as that of the host
    if (inp->byte_order == _MERRY_BYTE_ORDER_)
    {
        // the ordering is the same as that of the host
        // this makes life a lot easier
        // we can simply read the input bytes one by one to the VM's memory
        return merry_reader_byorder_same_i(inp);
    }
    // this indicates the byte ordering is not the same as that of the host, this will be a bit painful
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
    return inp;
failed:
    merry_destory_reader(inp);
    return RET_NULL;
}