#ifndef _PREPROCESSOR_SYMS_
#define _PREPROCESSOR_SYMS_

#include <unordered_map>
#include <string>

namespace masm
{
    namespace prepsyms
    {
        class PrepSyms
        {
            std::unordered_map<std::string, std::string> _syms_;
            public:
            PrepSyms() = default;

            void push_sym(std::string, std::string);
            std::string operator[](std::string);
        };
    };
};

#endif