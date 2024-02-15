#include "../utils/utils.hpp"

mlang::PosDetail mlang::extract_word(std::string str, std::string::iterator iter)
{
    // we first need to get the position of the first character of the word
    PosDetail detail;
    std::string::iterator temp = iter; // iter won't be changed
    if (iter == str.begin())
    {
        detail.start = 0;
        while (temp != str.end() && *temp != ' ')
        {
            temp++;
        }
        detail.end = static_cast<size_t>(temp - iter - 1);
        return detail; // word extracted
    }
    // if (iter == str.end())
    // {
    //     detail.end = 
    //     }
    if (*iter == ' ')
    {
        // if we point to a whitespace already, we return based on the situation
    }
}