#ifndef _EMIT_
#define _EMIT_

#include "codegen.hpp"

namespace masm
{
    namespace emit
    {
        class Emit
        {
            std::string out_file_name;
            std::vector<codegen::Instruction> instructions;
            std::vector<unsigned char> data;
            size_t entry;
            size_t str_len = 0;
            public:
            Emit() = default;

            Emit(codegen::Codegen&);

            void set_output_filename(std::string name)
            {
                out_file_name = name == ""? "a.mbin": name.ends_with(".mbin")? name: (name+=".mbin");
            }

            void emit();
        };
    };
};

#endif