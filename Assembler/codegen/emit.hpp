#ifndef _EMIT_
#define _EMIT_

#include "codegen.hpp"

namespace masm
{
    namespace emit
    {
        class Emit
        {
            std::string out_file_name = "a.mbin"; // default name
            std::vector<codegen::Instruction> instructions;
            std::vector<unsigned char> data;
            size_t entry;
            public:
            Emit() = default;

            Emit(codegen::Codegen&);

            void emit();
        };
    };
};

#endif