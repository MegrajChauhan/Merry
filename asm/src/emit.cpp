#include "emit.hpp"

void masm::Emit::emit(std::string output, std::string *epval, std::unordered_map<std::string, std::string> *tep, std::vector<std::string> *entry, std::vector<GenBinary> *_c, std::vector<mbyte_t> *_d, std::vector<mbyte_t> *_s)
{
    f.open(output, std::ios::out | std::ios::binary);
    if (!f.is_open())
    {
        note("Failed to open output file.");
        die(1);
    }
    eepe = epval;
    teepe = tep;
    entries = entry;
    code = _c;
    data = _d;
    str_data = _s;
}

void masm::Emit::add_header()
{
    f << 0x4D << 0x49 << 0x4E << 0x00 << 0x00 << 0x00 << 0x00 << 0x00; // we currently have no support for options
    ByteSwap b;
    b.val = code->size() * 8;
    for (auto v : b.b)
    {
        f << v;
    }
    // EAT
    
    // the SsT
    // For now, we only have data and string and hence 32 bytes at most
    f << 0x00 << 0x00 << 0x00 << 0x00 << 0x00 << 0x00 << 0x00;
    if (!str_data->empty())
        f << 32;
    else
        f << 16;
    
}
