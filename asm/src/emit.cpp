#include "emit.hpp"

void masm::Emit::emit(std::string output)
{
    f.open(output, std::ios::out | std::ios::binary);
    if (!f.is_open())
    {
        note("Failed to open output file.");
        exit(1);
    }
    analyze_eat();
    add_header();
    add_EAT();
    add_instructions();
    add_SsT();
    add_sections();
    if (gen_ST)
    {
        add_symd();
        add_ST();
    }
    f.close();
}

void masm::Emit::set_for_debug(bool ed, bool gST, bool _cd, bool _cdf)
{
    enable_dbg = ed;
    gen_ST = gST;
    cd = _cd;
    cdf = _cdf;
}

void masm::Emit::add_header()
{
    uint8_t header[8] = {'M', 'I', 'N', 0, 0, 0};
    header[6] = cdf ? 1 : 0;
    header[6] <<= 1;
    header[6] |= cd ? 1 : 0;
    header[6] <<= 1;
    header[6] = enable_dbg ? 1 : 0;
    header[7] |= gen_ST ? 1 : 0;
    for (auto v : header)
    {
        f << v;
    }
    ByteSwap b;
    b.val = code->size() * 8;
    for (int i = 7; i >= 0; i--)
    {
        f << b.b[i];
    }
    // EAT
    b.val = EAT_cont.size() * 8;
    for (int i = 7; i >= 0; i--)
    {
        f << b.b[i];
    }
    // the SsT
    // For now, we only have data and string and hence 32 bytes at most
    b.val = 16;
    if (!str_data->empty())
        b.val += 16;
    if (gen_ST)
        b.val += 16;
    for (int i = 7; i >= 0; i--)
    {
        f << b.b[i];
    }
    b.val = gen_ST ? ST->size() : 0;
    for (int i = 7; i >= 0; i--)
    {
        f << b.b[i];
    }
}

void masm::Emit::analyze_eat()
{
    size_t eepe_to_num = std::stoull(*eepe);
    for (auto _e : *entries)
    {
        auto _r = teepe->find(_e);
        size_t times = 0;
        if (_r != teepe->end())
            times = std::stoull(_r->second);
        else
            times = eepe_to_num;
        size_t addr = (*lbl_addr)[_e];
        for (int i = 0; i < times; i++)
            EAT_cont.push_back(addr);
    }
}

void masm::Emit::add_EAT()
{
    for (auto _e : EAT_cont)
    {
        ByteSwap b;
        b.val = _e;
        for (int i = 7; i >= 0; i--)
        {
            f << b.b[i];
        }
    }
}

void masm::Emit::add_instructions()
{
    for (auto _i : *code)
    {
        f << _i.bytes.b8;
        f << _i.bytes.b7;
        f << _i.bytes.b6;
        f << _i.bytes.b5;
        f << _i.bytes.b4;
        f << _i.bytes.b3;
        f << _i.bytes.b2;
        f << _i.bytes.b1;
    }
}

void masm::Emit::add_SsT()
{
    // for now we only have data and string so no debugging please
    // As for the debug sections, the assembler will have to define its own conventions on how
    // each section is structured which will take time so we avoid it for now
    // Let's just get a working assembler for now
    ByteSwap b;
    size_t j = data->size();
    for (int i = 7; i >= 0; i--)
    {
        uint8_t v = ((j >> (i * 8)) & 255);
        f << v;
    }
    b.val = 0;
    b.b[1] = 1; // RIM flag
    for (auto v : b.b)
        f << v;
    sections.push_back(data);
    if (!str_data->empty())
    {
        j = str_data->size();
        for (int i = 7; i >= 0; i--)
        {
            uint8_t v = ((j >> (i * 8)) & 255);
            f << v;
        }
        b.val = 0;
        b.b[1] = 1; // RIM flag
        b.b[2] = 1; // RAS flag
        for (auto v : b.b)
            f << v;
        sections.push_back(str_data);
    }
    if (gen_ST)
    {
        j = symd->size() * 16;
        for (int i = 7; i >= 0; i--)
        {
            uint8_t v = ((j >> (i * 8)) & 255);
            f << v;
        }
        b.b[0] = 2;
        for (auto v : b.b)
            f << v;
    }
}

void masm::Emit::add_sections()
{
    for (auto _s : sections)
    {
        for (auto _v : *_s)
        {
            f << _v;
        }
    }
}

void masm::Emit::add_ST()
{
    for (auto _c : *ST)
    {
        f << _c;
    }
}

void masm::Emit::add_symd()
{
    ByteSwap b;
    for (auto _e : *symd)
    {
        b.val = _e.first;
        for (int i = 7; i >= 0; i--)
            f << b.b[i];
        b.val = _e.second;
        for (int i = 7; i >= 0; i--)
            f << b.b[i];
    }
}

/**
 * For now, we may have emitted the multiple features of the file format but they will be in
 * place as needed.
 */
