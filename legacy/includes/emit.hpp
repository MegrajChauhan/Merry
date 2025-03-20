#ifndef _EMIT_
#define _EMIT_

#include <unordered_map>
#include <vector>
#include <filesystem>
#include <fstream>
#include "context.hpp"

/**
 * GIANT NOTE:
 *
 * The format allows for four types of sections: `data`, `others`, `symd`, and `info`. For now, this assembler will
 * only use `data` and `symd`. While future versions might expand to include the other types, they are not currently necessary.
 * The structure and content of these sections are determined by the assembler, meaning that the creators of assemblers have full
 * control over how these sections are organized. Debuggers may need to follow these conventions, as the assembler can pass valuable
 * information through these sections.
 *
 * Example: Using the `info` section
 * ---------------------------------
 * Let's say the assembler utilizes the `info` section in three different ways:
 * 1. To provide details about the files involved in the assembly.
 * 2. To indicate which instructions correspond to which file.
 * 3. To convey information about the assembler itself or other similar data.
 *
 * To facilitate this, the assembler could define a convention where the first byte of the section acts as metadata, indicating what the
 * section contains. By reserving one byte for this purpose, the assembler can define up to 255 different types of information within the `info` section.
 * The same principle could be applied to the `others` section. However, in this case, the assembler might instruct the VM to read it directly, allowing
 * the debugger to communicate with the VM to access the contents. Though this approach is possible, it might not be ideal.
 */

namespace masm
{
    // no matter the native endinness, make sure it is big-endianness
    union ByteSwap
    {
        mbyte_t b[8]; // access in opposite manner
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

        std::vector<mbyte_t> *ST;
        std::unordered_map<size_t, size_t> *symd;

        bool enable_dbg = false, gen_ST = false, cd = false, cdf = false;

    public:
        Emit() = default;

        void init_for_debug(std::vector<mbyte_t> *st, std::unordered_map<size_t, size_t> *_symd);

        void set_for_debug(bool ed, bool gST, bool _cd, bool _cdf);

        void emit(std::string output, std::string *epval, std::unordered_map<std::string, std::string> *tep, std::vector<std::string> *entry, std::vector<GenBinary> *_c, std::vector<mbyte_t> *_d, std::vector<mbyte_t> *_s, std::unordered_map<std::string, size_t> *lbaddr);

        void add_header();

        void add_EAT();

        void add_SsT();

        void add_instructions();

        void analyze_eat();

        void add_sections();

        void add_symd();

        void add_ST();
    };
};

#endif