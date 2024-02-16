#ifndef _ERROR_
#define _ERROR_

#include <string>
#include <queue>

namespace mlang
{
    enum Error_level
    {
        FATAL,
        WARNING,
        NOTE,
    };

    enum ErrorContext
    {
        READING,
        LEXING,
        PARSING,
    };

    // the inheriting class may use the above defined constructs or not
    template <typename T>
    class Error
    {
    protected:
        std::queue<T> _errors_; // here is where the errors remain
    public:

        void add_error(T error);

        T get_next_error();

        bool has_error();
    };
};

#endif