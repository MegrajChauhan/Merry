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

        size_t ISS = 0;   // Instruction Section Size
        size_t EAT_S = 0; // EAT Size
        size_t SsT_S = 0; // SsT Size
        size_t ST_S = 0;  // ST size

        std::vector<size_t> EAT_cont;
        std::vector<std::pair<size_t, size_t>> SsT_cont; // will need in the future
        std::vector<std::vector<mbyte_t> *> sections;

        std::unordered_map<std::string, size_t> *lbl_addr;

    public:
        Emit() = default;

        void emit(std::string output, std::string *epval, std::unordered_map<std::string, std::string> *tep, std::vector<std::string> *entry, std::vector<GenBinary> *_c, std::vector<mbyte_t> *_d, std::vector<mbyte_t> *_s, std::unordered_map<std::string, size_t> *lbaddr);

        void add_header();

        void add_EAT();

        void add_SsT();

        void add_instructions();

        void analyze_eat();

        void add_sections();
    };
};

#endif