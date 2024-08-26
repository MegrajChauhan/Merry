#ifndef _EMIT_
#define _EMIT_

#include <unordered_map>
#include <vector>
#include <filesystem>
#include <fstream>
#include "context.hpp"

namespace masm
{
    // no matter the native endinness, make sure it is big-endianness
    union ByteSwap
    {
        mbyte_t b[8];
        size_t val;
    };

    class Emit
    {
        std::string *eepe;
        std::unordered_map<std::string, std::string> *teepe;
        std::vector<std::string> *entries;
        std::vector<GenBinary> *code;
        std::vector<mbyte_t> *data;
        std::vector<mbyte_t> *str_data;
        std::fstream f;

    public:
        Emit() = default;

        void emit(std::string output, std::string *epval, std::unordered_map<std::string, std::string> *tep, std::vector<std::string> *entry, std::vector<GenBinary> *_c, std::vector<mbyte_t> *_d, std::vector<mbyte_t> *_s);

        void add_header();
    };
};

#endif