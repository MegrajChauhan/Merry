#ifndef _PREPROCESSOR_
#define _PREPROCESSOR_

#include <vector>
#include "reader.hpp"
#include "preprocessor_syms.hpp"

namespace masm
{
    namespace prep
    {
        class Prep
        {
            prepsyms::PrepSyms syms;
            reader::Reader reader;
            std::string contents;

        public:
            Prep(std::string);

            void prep();

            std::string get_contents();
        };
    };
};

#endif