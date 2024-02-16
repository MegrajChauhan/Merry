#include "../utils/utils.hpp"

mlang::PosDetail mlang::extract_word(std::string str, std::string::iterator iter)
{
    // we first need to get the position of the first character of the word
    PosDetail detail;
    std::string first_half = str.substr(0, std::distance(str.begin(), iter));
    std::string second_half = str.substr(std::distance(str.begin(), iter));
    detail.start = first_half.find_first_of(' ');
    detail.end = second_half.find_first_of(' ');
    return detail;
}

mlang::PosDetail mlang::extract_word(std::string str, size_t pos)
{
    return extract_word(str, str.begin() + pos);
}

std::string mlang::highlight_error(std::string str, mlang::PosDetail detail)
{
    str.append(_CCODE_RED, detail.start, std::string(_CCODE_RED).size());
    str.append(_CCODE_RESET, detail.end, std::string(_CCODE_RESET).size());
    return str;
}