/*
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _EMIT_
#define _EMIT_

#include <unordered_map>
#include <vector>
#include <filesystem>
#include <fstream>
#include "error.hpp"
#include "merry_config.h"

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
        uint8_t b[8]; // access in opposite manner
        size_t val;
    };

    struct GenBinary
    {
        union
        {
            struct
            {
#if _MERRY_ENDIANNESS_ == _MERRY_LITTLE_ENDIAN_
                uint8_t b8, b7, b6, b5, b4, b3, b2, b1;
#else
                uint8_t b1, b2, b3, b5, b6, b4, b7, b8;
#endif
            } bytes;
            unsigned long long full = 0;
        };
    };


    class Emit
    {
        std::fstream f;

        size_t ISS = 0;   // Instruction Section Size
        size_t EAT_S = 0; // EAT Size
        size_t SsT_S = 0; // SsT Size
        size_t ST_S = 0;  // ST size

        std::vector<size_t> EAT_cont;
        std::vector<std::pair<size_t, size_t>> SsT_cont; // will need in the future
        std::vector<std::vector<uint8_t> *> sections;

        bool enable_dbg = false, gen_ST = false, cd = false, cdf = false;

    public:
        std::vector<uint8_t> *ST;
        std::unordered_map<size_t, size_t> *symd;
        std::unordered_map<std::string, size_t> *lbl_addr;
        std::string *eepe;
        std::unordered_map<std::string, std::string> *teepe;
        std::vector<std::string> *entries;
        std::vector<GenBinary> *code;
        std::vector<uint8_t> *data;
        std::vector<uint8_t> *str_data;

        Emit() = default;

        void set_for_debug(bool ed, bool gST, bool _cd, bool _cdf);

        void emit(std::string output);

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