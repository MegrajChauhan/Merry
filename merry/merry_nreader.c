#include "internals/merry_nreader.h"

_MERRY_INTERNAL_ void merry_reader_give_mem_to_os(mptr_t *addrs, msize_t count)
{
    for (msize_t i = 0; i < count; i++)
    {
        _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(addrs[i]);
    }
}

_MERRY_INTERNAL_ mret_t merry_reader_get_mem_from_os(mptr_t **addrs, msize_t count)
{
    // This will get us memory pages from the Host OS directly
    // On failure, we can do nothing but exit
    *addrs = (mptr_t *)malloc(sizeof(mptr_t) * count);
    if (*addrs == NULL)
        return RET_NULL;
    for (msize_t i = 0; i < count; i++)
    {
        if (((*addrs)[0] = _MERRY_MEMORY_PGALLOC_MAP_PAGE_) == _MERRY_RET_GET_ERROR_)
        {
            merry_reader_give_mem_to_os(*addrs, i);
            rlog("Internal Error: Couldn't receive memory from the OS.\n", NULL);
            free(*addrs);
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ void merry_reader_return_all_mem(MerryReader *r)
{
    if (r->inst.inst_page_count > 0)
    {
        merry_reader_give_mem_to_os(r->inst.instructions, r->inst.inst_page_count);
        free(r->inst.instructions);
    }
    if (r->data_page_count > 0)
    {
        merry_reader_give_mem_to_os(r->data, r->data_page_count);
        free(r->data);
    }
}

MerryReader *merry_init_reader(mcstr_t filename)
{
    MerryReader *r = (MerryReader *)malloc(sizeof(MerryReader));
    if (r == NULL)
        return RET_NULL;
    if ((r->f = fopen(filename, "rb")) == NULL)
    {
        rlog("Couldn't open given input file %s.\n", filename);
        free(r);
        return RET_NULL;
    }
    r->de_flag = mfalse;
    r->ste_flag = mfalse;
    r->data_len = 0;
    r->data_page_count = 0;
    r->eat.eat_entry_count = 0;
    r->eat.eat_len = 0;
    r->inst.inst_page_count = 0;
    r->inst.inst_section_len = 0;
    r->sst.sst_entry_count = 0;
    r->sst.sst_len = 0;
    r->st.st_entry_count = 0;
    r->st.st_len = 0;
    r->sst.symd_section_provided = mfalse;
    return r;
}

void merry_destroy_reader(MerryReader *r)
{
    // Reader will not free everything
    // f is closed after reading
    if (r->eat.EAT != NULL)
        free(r->eat.EAT);
    if (r->sst.sections != NULL)
        free(r->sst.sections);
    if (r->st.st_entries != NULL)
    {
        for (msize_t i = 0; i < r->st.st_entry_count; i++)
        {
            if (r->st.st_entries[i] != NULL)
                free(r->st.st_entries[i]);
        }
        free(r->st.st_entries);
    }
    free(r);
}

mret_t merry_reader_is_file_fit_to_read(MerryReader *r)
{
    fseek(r->f, SEEK_SET, SEEK_END);
    size_t len = ftell(r->f);
    rewind(r->f);
    r->flen = len;
    if (surelyF(len < _MERRY_MIN_INPFILE_LEN_))
        return RET_FAILURE;
    return RET_SUCCESS;
}

mret_t merry_reader_read_header(MerryReader *r)
{
    mbyte_t header[_MERRY_MIN_INPFILE_LEN_];
    fread(header, 1, _MERRY_MIN_INPFILE_LEN_, r->f); // shouldn't fail
    // Now parse the bloody thing
    if (header[0] != 0x4D && header[1] != 0x49 && header[2] != 0x4E)
    {
        rlog("Unknown file format: Requires Merry's file format.\n", NULL);
        return RET_FAILURE;
    }
    // reading the flags
    if (header[_MERRY_STE_FLAG_] == 0x01)
        r->ste_flag = mtrue;
    if (header[_MERRY_DE_FLAG_] == 0x01)
        r->de_flag = mtrue;
    // The various section size
    mqword_t tmp = 0;
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
    _MERRY_GET_LITTLE_ENDIAN_(tmp, header, 8)
#else
    tmp = *(mqptr_t)(header + 8);
#endif
    r->inst.inst_section_len = tmp;
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
    _MERRY_GET_LITTLE_ENDIAN_(tmp, header, 16)
#else
    tmp = *(mqptr_t)(header + 16);
#endif
    r->eat.eat_len = tmp;
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
    _MERRY_GET_LITTLE_ENDIAN_(tmp, header, 24)
#else
    tmp = *(mqptr_t)(header + 24);
#endif
    r->sst.sst_len = tmp;
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
    _MERRY_GET_LITTLE_ENDIAN_(tmp, header, 32)
#else
    tmp = *(mqptr_t)(header + 32);
#endif
    r->st.st_len = tmp;

    // make couple of checks
    if ((r->inst.inst_section_len + r->eat.eat_len + r->sst.sst_len + r->st.st_len) > r->flen)
    {
        rlog("The provided header data doesn't match with the file's contents.", NULL);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

mret_t merry_reader_validate_header_info(MerryReader *r)
{
    // check if the different header lengths are correct
    if (r->inst.inst_section_len == 0)
    {
        rlog("There are no instructions to read.\n", NULL);
        return RET_FAILURE;
    }
    if ((r->inst.inst_section_len % 8) != 0)
    {
        rlog("The instruction section is not a aligned[Must be 8-byte aligned]\n", NULL);
        return RET_FAILURE;
    }
    if ((r->sst.sst_len % 16) != 0)
    {
        rlog("The sections table doesn't have proper, complete entries.\n", NULL);
        return RET_FAILURE;
    }
    if ((r->eat.eat_len % 8) != 0)
    {
        rlog("The entries in the EAT table are not aligned[Each entry is required to be 8-byte aligned]..\n", NULL);
        return RET_FAILURE;
    }
    if (r->st.st_len == 0 && r->ste_flag == mtrue)
    {
        rlog("STE flag provided but the ST is empty: Ignoring the flag.\n", NULL);
        r->ste_flag = mfalse;
    }
    if (r->sst.sst_len == 0 && r->de_flag == mtrue)
    {
        rlog("DE flag provided by the SsT is empty: Ignoring the flag[No Data Provided either].\n", NULL);
        r->de_flag = mfalse;
        r->data_len = 0;
    }
    return RET_SUCCESS;
}

mret_t merry_reader_read_eat(MerryReader *r)
{
    // Just read the Entry Address Table
    msize_t num_of_entries = r->eat.eat_len / _MERRY_EAT_PER_ENTRY_LEN_;
    if (num_of_entries == 0)
    {
        rlog("Note: No entries found in the EAT. Defaulting to: one core from address 0x0000000000000000.\n", NULL);
        r->eat.eat_entry_count = 1;
        r->eat.EAT = (maddress_t *)malloc(8);
        if (r->eat.EAT == NULL)
        {
            // we failed even this?!! Embarrassing
            rlog("Internal Error: Failed to allocate memory.\n", NULL);
            return RET_FAILURE;
        }
        r->eat.EAT[0] = 0;
        return RET_SUCCESS;
    }
    r->eat.EAT = (maddress_t *)malloc(8 * num_of_entries);
    mbyte_t eat_conts[r->eat.eat_len];
    fread(eat_conts, 1, r->eat.EAT, r->f); // hoping it doesn't fail
    if (r->eat.EAT == NULL)
    {
        // we failed even this?!! Embarrassing
        rlog("Internal Error: Failed to allocate memory.\n", NULL);
        return RET_FAILURE;
    }
    for (msize_t i = 0, j = 0; i < num_of_entries; i++)
    {
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
        _MERRY_GET_LITTLE_ENDIAN_(r->eat.EAT[i], eat_conts, j)
#else
        r->eat.EAT[i] = *(mqptr_t)(eat_conts + j);
#endif
        j += 8;
    }
    r->eat.eat_entry_count = num_of_entries;
    return RET_SUCCESS;
}

mret_t merry_reader_read_instructions(MerryReader *r)
{
    // Necessary checks are already made before
    /// NOTE: The VM will not check if the addresses in the EAT are valid or not.
    msize_t inst_count = r->inst.inst_section_len / 8;
    msize_t number_of_pages = inst_count / _MERRY_MEMORY_QS_PER_PAGE_;
    msize_t extra_addrs = inst_count % _MERRY_MEMORY_QS_PER_PAGE_;
    r->inst.inst_page_count = number_of_pages + (extra_addrs > 0 ? 1 : 0);
    if (merry_reader_get_mem_from_os(&r->inst.instructions, r->inst.inst_page_count) == RET_FAILURE)
        return RET_FAILURE;
    for (msize_t i = 0; i < number_of_pages; i++)
    {
        if (fread(r->inst.instructions[i], 8, _MERRY_MEMORY_QS_PER_PAGE_, r->f) != _MERRY_MEMORY_QS_PER_PAGE_)
        {
            rlog("Internal Error: Failed to read input file.\n", NULL);
            merry_reader_return_all_mem(r);
            return RET_FAILURE;
        }
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
        for (msize_t j = 0; j < _MERRY_MEMORY_QS_PER_PAGE_; j++)
        {
            mqword_t current = r->inst.instructions[i][j];
            mqword_t inverted = 0;
            inverted = current >> 56;
            inverted <<= 8;
            inverted |= ((current >> 48) & 255);
            inverted <<= 8;
            inverted |= ((current >> 40) & 255);
            inverted <<= 8;
            inverted |= ((current >> 32) & 255);
            inverted <<= 8;
            inverted |= ((current >> 24) & 255);
            inverted <<= 8;
            inverted |= ((current >> 16) & 255);
            inverted <<= 8;
            inverted |= ((current >> 8) & 255);
            inverted <<= 8;
            inverted |= ((current) & 255);
            r->inst.instructions[i][j] = inverted;
        }
#endif
    }
    // Now read the remaining instructions
    if (extra_addrs > 0)
    {
        if (fread(r->inst.instructions[number_of_pages], 8, extra_addrs, r->f) != extra_addrs)
        {
            rlog("Internal Error: Failed to read input file.\n", NULL);
            merry_reader_return_all_mem(r);
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

mret_t merry_reader_read_sst(MerryReader *r)
{
    msize_t sst_entry_count = r->sst.sst_len / _MERRY_SST_PER_ENTRY_LEN_;
    // We are still well within len
    r->sst.sections = (MerrySection *)malloc(sizeof(MerrySection) * sst_entry_count);
    if (r->sst.sections == NULL)
        return RET_FAILURE;
    msize_t total_len = 0;
    for (msize_t i = 0; i < sst_entry_count; i++)
    {
        mbyte_t entry[_MERRY_SST_PER_ENTRY_LEN_];
        if (fread(entry, 1, _MERRY_SST_PER_ENTRY_LEN_, r->f) != _MERRY_SST_PER_ENTRY_LEN_)
        {
            rlog("Internal Error: Couldn't read SsT from input.\n", NULL);
            free(r->sst.sections);
            return RET_FAILURE;
        }
        
    }
    return RET_SUCCESS;
}
