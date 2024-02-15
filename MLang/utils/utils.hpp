#ifndef _UTILS_
#define _UTILS_

#include <string>

namespace mlang
{
    // given an iterator, get the word that the iterator is inside of in the string 'str'
    struct PosDetail
    {
        size_t start;
        size_t end;
    };

    PosDetail extract_word(std::string str, std::string::iterator iter);
    PosDetail extract_word(std::string str, size_t pos);
};

#endif