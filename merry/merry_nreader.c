#include "merry_nreader.h"

_MERRY_INTERNAL_ void merry_reader_give_mem_to_os(mptr_t *addrs, msize_t count)
{
    for (msize_t i = 0; i < count; i++)
    {
        if (addrs[i] != NULL)
            _MERRY_MEMORY_PGALLOC_UNMAP_PAGE_(addrs[i]);
    }
}

_MERRY_INTERNAL_ mptr_t *merry_reader_get_mem_from_os(msize_t count)
{
    // This will get us memory pages from the Host OS directly
    // On failure, we can do nothing but exit
    if (count == 1)
    {
        mptr_t res = _MERRY_MEMORY_PGALLOC_MAP_PAGE_;
        if (res == _MERRY_RET_GET_ERROR_)
            return NULL;
        return res;
    }
    mptr_t *addrs = (mptr_t *)malloc(sizeof(mptr_t) * count);
    if (addrs == NULL)
        return NULL;
    for (msize_t i = 0; i < count; i++)
    {
        if (((addrs)[i] = _MERRY_MEMORY_PGALLOC_MAP_PAGE_) == _MERRY_RET_GET_ERROR_)
        {
            merry_reader_give_mem_to_os(addrs, i);
            rlog("Internal Error: Couldn't receive memory from the OS.\n", NULL);
            free(addrs);
            return NULL;
        }
    }
    return addrs;
}

_MERRY_INTERNAL_ void merry_reader_return_all_mem(MerryReader *r)
{
    if (r->inst.inst_page_count > 0)
    {
        merry_reader_give_mem_to_os((void *)r->inst.instructions, r->inst.inst_page_count);
        free(r->inst.instructions);
    }
    if (r->data_page_count > 0)
    {
        merry_reader_give_mem_to_os((void *)r->data, r->data_page_count);
        free(r->data);
    }
}

_MERRY_INTERNAL_ inline msize_t merry_reader_addr_to_pg_index(maddress_t addr)
{
    return addr / _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
}

MerryReader *merry_init_reader(mcstr_t filename)
{
    MerryReader *r = (MerryReader *)malloc(sizeof(MerryReader));
    if (r == NULL)
        return RET_NULL;
    if ((r->f = fopen(filename, "rb")) == NULL)
    {
        rlog("Couldn't open given input file %s. Is it a directory? Are you sure it exists? Is it a path mistake?\n", filename);
        free(r);
        return RET_NULL;
    }
    r->de_flag = mfalse;
    r->ste_flag = mfalse;
    r->dfe_flag = mfalse;
    r->dfw_flag = mfalse;
    r->data_len = 0;
    r->data_page_count = 0;
    r->eat.eat_entry_count = 0;
    r->eat.eat_len = 0;
    r->inst.inst_page_count = 0;
    r->inst.inst_section_len = 0;
    r->sst.sst_entry_count = 0;
    r->sst.sst_len = 0;
    r->st.st_len = 0;
    r->sst.symd_section_provided = mfalse;
    r->sym_count = 0;
    r->syms = NULL;
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
    if (r->st.st_data != NULL)
    {
        free(r->st.st_data);
    }
    if (r->syms != NULL)
    {
        free(r->syms);
    }
    free(r);
}

mret_t merry_reader_read_file(MerryReader *r)
{
    if (merry_reader_is_file_fit_to_read(r) == RET_FAILURE)
        return RET_FAILURE;
    if (merry_reader_read_header(r) == RET_FAILURE)
        return RET_FAILURE;
    if (merry_reader_validate_header_info(r) == RET_FAILURE)
        return RET_FAILURE;
    if (merry_reader_read_eat(r) == RET_FAILURE)
        return RET_FAILURE;
    if (merry_reader_read_instructions(r) == RET_FAILURE)
        goto failed;
    if (r->sst.sst_len != 0 && merry_reader_read_sst(r) == RET_FAILURE)
        goto failed;
    if (r->sst.sst_len != 0 && merry_reader_read_sections(r) == RET_FAILURE)
        goto failed;
    if (r->ste_flag == mtrue && r->st.st_len > 0 && merry_reader_read_st(r) == RET_FAILURE)
        goto failed;
    return RET_SUCCESS;
failed:
    merry_reader_return_all_mem(r);
    return RET_FAILURE;
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
    if (ste_(header) == 0x01)
        r->ste_flag = mtrue;
    if (de_(header) == 0x01)
        r->de_flag = mtrue;
    if (dfe_(header) == 0x01)
        r->dfe_flag = mtrue;
    if (dfw_(header) == 0x01)
        r->dfw_flag = mtrue;
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
        rlog("The provided header data doesn't match with the file's contents.\n", NULL);
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
    if (r->eat.EAT == NULL)
    {
        rlog("Internal Error: Failed to allocate memory.\n", NULL);
        return RET_FAILURE;
    }
    mbyte_t eat_conts[r->eat.eat_len];
    fread(eat_conts, 1, r->eat.eat_len, r->f); // hoping it doesn't fail
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

mret_t merry_reader_read_inst_page(MerryReader *r, mqptr_t store_in, msize_t pg_ind)
{
    msize_t pos = r->inst.start_offset + pg_ind * _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    fseek(r->f, pos, SEEK_SET);
    if (pg_ind == (r->inst.inst_page_count - 1))
    {
        msize_t extra_addrs = r->inst.inst_section_len - (pg_ind * _MERRY_MEMORY_ADDRESSES_PER_PAGE_) / 8;
        if (fread(store_in, 8, extra_addrs, r->f) != extra_addrs)
        {
            rlog("Internal Error: Failed to read input file.\n", NULL);
            return RET_FAILURE;
        }
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
        for (msize_t j = 0; j < extra_addrs; j++)
        {
            mqword_t current = store_in[j];
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
            store_in[j] = inverted;
        }
#endif
    }
    else
    {
        if (fread(store_in, 8, _MERRY_MEMORY_QS_PER_PAGE_, r->f) != _MERRY_MEMORY_QS_PER_PAGE_)
        {
            rlog("Internal Error: Failed to read input file.\n", NULL);
            return RET_FAILURE;
        }
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
        for (msize_t j = 0; j < _MERRY_MEMORY_QS_PER_PAGE_; j++)
        {
            mqword_t current = store_in[j];
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
            store_in[j] = inverted;
        }
#endif
    }
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
    r->inst.instructions = (mqptr_t *)calloc(r->inst.inst_page_count, 8);
    if (!r->inst.instructions)
        return RET_FAILURE;
    r->inst.start_offset = ftell(r->f);
    for (msize_t i = 0; i < r->eat.eat_entry_count; i++)
    {
        r->inst.instructions[i] = merry_reader_get_mem_from_os(1);
        if (!r->inst.instructions[i])
        {
            merry_reader_give_mem_to_os(r->inst.instructions, r->inst.inst_page_count);
            return RET_FAILURE;
        }
        if (merry_reader_read_inst_page(r, r->inst.instructions[i], merry_reader_addr_to_pg_index(r->eat.EAT[i])) == RET_FAILURE)
            return RET_FAILURE;
    }
    return RET_SUCCESS;
}

// we read this section only if SsT len is greater than 0 in the first place
mret_t merry_reader_read_sst(MerryReader *r)
{
    msize_t pos = r->inst.start_offset + r->inst.inst_page_count * _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    fseek(r->f, pos, SEEK_SET);
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
            return RET_FAILURE;
        }
        msize_t size = 0;
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
        _MERRY_GET_LITTLE_ENDIAN_(size, entry, 0)
#else
        size = *(mqptr_t)(eat_conts + j);
#endif
        if ((size % 8) != 0)
        {
            rlog("Error: Sections must be 8-byte aligned.\n", NULL);
            return RET_FAILURE;
        }
        msection_t _t = entry[8] & 3;
        mbool_t rim = entry[9] & 0x1, ras = entry[10] & 0x1;
        msize_t stind = entry[12];
        stind <<= 8;
        stind |= entry[13];
        stind <<= 8;
        stind |= entry[14];
        stind <<= 8;
        stind |= entry[15];
        total_len += size; // add to the total length
        if (_t == _SYMD)
            r->sst.symd_section_provided = mtrue;
        r->sst.sections[i].ras = ras;
        r->sst.sections[i].rim = (_t == _INFO || _t == _SYMD) ? mfalse : rim;
        r->sst.sections[i].section_len = size;
        r->sst.sections[i].st_index = stind;
        r->sst.sections[i].type = _t;
        if (r->sst.sections[i].rim == mtrue)
            r->data_len += size;
    }
    r->sst.sst_entry_count = sst_entry_count;
    // make a check here
    if ((r->inst.inst_section_len + r->eat.eat_len + r->sst.sst_len + r->st.st_len + total_len) > r->flen)
    {
        rlog("The section's total size is greater than the file itself.\n", NULL);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

mret_t merry_reader_read_data_page(MerryReader *r, msize_t pg_ind, MerrySection *s)
{
    fseek(r->f, r->affordable_offsets[pg_ind], SEEK_SET);
    if (pg_ind == (r->data_page_count - 1))
    {
        msize_t extra_addrs = r->data_len - (pg_ind * _MERRY_MEMORY_ADDRESSES_PER_PAGE_);
        if (fread(r->data[pg_ind], 1, extra_addrs, r->f) != extra_addrs)
        {
            rlog("Internal Error: Failed to read input file.\n", NULL);
            return RET_FAILURE;
        }
        if (s->ras != mtrue)
        {
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
            for (msize_t j = 0; j < extra_addrs; j++)
            {
                mqword_t current = r->data[pg_ind][j];
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
                r->data[pg_ind][j] = inverted;
            }
#endif
        }
    }
    else
    {
        if (fread(r->data[pg_ind], 1, _MERRY_MEMORY_ADDRESSES_PER_PAGE_, r->f) != _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
        {
            rlog("Internal Error: Failed to read input file.\n", NULL);
            return RET_FAILURE;
        }
        if (s->ras != mtrue)
        {
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
            for (msize_t j = 0; j < _MERRY_MEMORY_ADDRESSES_PER_PAGE_; j++)
            {
                mqword_t current = r->data[pg_ind][j];
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
                r->data[pg_ind][j] = inverted;
            }
        }
#endif
    }
}

mret_t merry_reader_read_sections(MerryReader *r)
{
    // based on the information from the SsT, read the various sections
    // Mostly every readable section is just data
    // symd is directly stored by the reader
    msize_t data_count = r->data_len / 8;
    msize_t number_of_pages = data_count / _MERRY_MEMORY_QS_PER_PAGE_;
    msize_t extra_addrs = data_count % _MERRY_MEMORY_QS_PER_PAGE_;
    r->data_page_count = number_of_pages + (extra_addrs > 0 ? 1 : 0);
    r->affordable_offsets = malloc(8 * number_of_pages);
    if (!r->affordable_offsets)
        return RET_FAILURE;
    r->data = (mbptr_t *)calloc(r->data_page_count, 8);
    if (!r->data)
    {
        free(r->affordable_offsets);
        return RET_FAILURE;
    }
    msize_t current_index = 0, current_off = 0;
    msize_t _pages_of_data_read_ = 0;
    for (msize_t i = 0; i < r->sst.sst_entry_count; i++)
    {
        MerrySection current_section = r->sst.sections[i];
        if (current_section.rim == mfalse && current_section.type != _SYMD && current_section.type != _INFO)
        {
            // skip the section
            mbyte_t _t[current_section.section_len];
            if (fread(_t, 1, current_section.section_len, r->f) != current_section.section_len)
            {
                rlog("Internal Error: Failed to read data.\n", NULL);
                return RET_FAILURE;
            }
            continue;
        }
        switch (current_section.type)
        {
            // we do not read the info section
        case _SYMD:
        {
            // Each entry in this section is in the format: ind = address
            // first 8 bytes hold the address of the symbol and the second 8 bytes hold the index
            // Both the address and the index should be in Big Endian format
            register msize_t entries = current_section.section_len / _MERRY_SYMD_PER_ENTRY_LEN_;
            r->sym_count += entries;
            r->syms = (MerrySymbol *)realloc(r->syms, sizeof(MerrySymbol) * r->sym_count);
            if (r->syms == NULL)
            {
                rlog("Internal Error: Failed to allocate memory to store the symbols.\n", NULL);
                return RET_FAILURE;
            }
            for (msize_t j = 0; j < entries; j++)
            {
                mbyte_t entry[_MERRY_SYMD_PER_ENTRY_LEN_];
                if (fread(entry, 1, _MERRY_SYMD_PER_ENTRY_LEN_, r->f) != _MERRY_SYMD_PER_ENTRY_LEN_)
                {
                    rlog("Internal Error: Failed to read data.\n", NULL);
                    return RET_FAILURE;
                }
                register maddress_t address = 0, index = 0;
#if _MERRY_BYTE_ORDER_ == _MERRY_LITTLE_ENDIAN_
                _MERRY_GET_LITTLE_ENDIAN_(address, entry, 0)
                _MERRY_GET_LITTLE_ENDIAN_(index, entry, 8)
#else
                    address = *(mqptr_t)(entry);
                    index = *(mqptr_t)(entry + 8);
#endif
                r->syms[j].address = address;
                r->syms[j].index = index;
            }
            break;
        }
        case _OTHER:
        case _DATA:
        {
        }
        }
    }
    return RET_SUCCESS;
}

mret_t merry_reader_read_st(MerryReader *r)
{
    // Just read everything remaining into the memory
    // This section contains string that is indexed by the ST indexes everywhere.
    // Each string needs to be NULL terminated
    r->st.st_data = (mbptr_t)malloc((r->st.st_len));
    if (r->st.st_data == NULL)
    {
        rlog("Internal Error: Unable to allocate memory for ST.\n", NULL);
        return RET_FAILURE;
    }
    if (fread(r->st.st_data, 1, r->st.st_len, r->f) != r->st.st_len)
    {
        rlog("Internal Error: Couldn't read the ST.\n", NULL);
        free(r->st.st_data);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

mbptr_t merry_reader_get_symbol(MerryReader *r, maddress_t addr)
{
    if (r->ste_flag == mfalse || r->sst.symd_section_provided == mfalse)
        return RET_NULL;
    for (msize_t i = 0; i < r->sym_count; i++)
    {
        if (r->syms[i].address == addr)
            return (r->st.st_data + r->syms[i].index);
    }
    return RET_NULL;
}
