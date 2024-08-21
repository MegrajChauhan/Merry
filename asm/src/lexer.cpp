#include "lexer.hpp"

void masm::Lexer::setup_lexer(std::shared_ptr<std::string> conts, std::shared_ptr<std::string> path)
{
    fileconts = conts;
    curr = fileconts.get()->begin();
    end = fileconts.get()->end();
    offset = 0;
    line = 1;
    col = 0;
    filepath = path;
}

void masm::Lexer::consume()
{
    if (curr == end)
        return;
    if (c == '\n')
    {
        line++;
        col = 0;
    }
    curr++;
    col++;
    offset++;
}

char masm::Lexer::peek(size_t by)
{
    return (curr == end) ? '\0' : ((curr + by) >= end) ? '\0'
                                                       : *(curr + by);
}

void masm::Lexer::rid_until(char until)
{
    while (curr != end)
    {
        if (c != until)
            consume();
        else
            break;
    }
    // we don't consume the next character
}

std::string masm::Lexer::extract_line()
{
    std::string::iterator st, ed;
    st = ed = curr;
    if (*st == '\n')
        st--;
    while (st != fileconts.get()->begin() && *st != '\n')
        st--;

    while (ed != end && *ed != '\n')
        ed++;
    return std::string(st + 1, ed);
}

std::optional<masm::Token> masm::Lexer::next_token()
{
    Token t;
    t.type = MB_C;
    while (curr != end)
    {
        if (c == ';')
        {
            if (peek() == ';')
            {
                // a comment, so get rid of it
                rid_until('\n');
                consume();
            }
            else
            {
                err(*filepath.get(), line, col, col + 1, _lexing, syntaxerr, ERR_STR, "This is not how you use comments.", extract_line());
                return {};
            }
        }
        else if (std::isspace(c))
        {
            while (std::isspace(c))
                consume();
        }
        else if (std::isalpha(c) || c == '_')
        {
            t = group_word();
            break;
        }
        else if (std::isdigit(c) || (c == '-' && std::isdigit(peek())))
        {
            auto res = group_num();
            if (!res.has_value())
                return {};
            t = res.value();
            break;
        }
        else if (c == '\"')
        {
            auto res = gather_quote_string();
            if (!res.has_value())
                return {};
            t = res.value();
            break;
        }
        else if (c == ',')
        {
            consume();
        }
        else
        {
            if (c == '.' && ignore_dots)
            {
                t.col = col;
                t.line = line;
                t.type = OPER_DOT;
                consume();
                break;
            }
            else
            {
                err(*filepath.get(), line, col, col + 1, _lexing, invalidtok, ERR_STR, "Cannot build a token from this.", extract_line());
                return {};
            }
        }
    }
    if (t.type == MB_C)
        return {};
    return std::make_optional<Token>(t);
}

masm::Token masm::Lexer::group_word()
{
    std::string wrd;
    size_t st = col;
    while (std::isalnum(c) || c == '_')
    {
        wrd += c;
        consume();
    }
    auto res = iden_map.find(wrd);
    Token t;
    t.col = st;
    t.line = line;
    if (res == iden_map.end())
    {
        t.type = IDENTIFIER;
        t.val = wrd;
    }
    else
        t.type = res->second;
    return t;
}

std::optional<masm::Token> masm::Lexer::group_num()
{
    Token t;
    int d_count = 0;
    std::string val;
    size_t st = col;
    if (c == '-')
    {
        val += c;
        consume();
    }
    while (std::isdigit(c) || c == '.')
    {
        if (c == '.')
        {
            if (ignore_dots)
                break;
            d_count++;
        }
        if (d_count > 1)
        {
            err(*filepath.get(), line, st, col, _lexing, syntaxerr, ERR_STR, "More than one '.' are not allowed.", extract_line());
            return {};
        }
        val += c;
        consume();
    }
    if (d_count > 0)
    {
        if (val.ends_with('.'))
            // we will be forgiving and add a '0'
            val += '0';
        t.type = NUM_FLOAT;
    }
    else
        t.type = NUM_INT;
    t.col = st;
    t.line = line;
    t.val = val;
    return std::make_optional<Token>(t);
}

std::string masm::Lexer::get_a_group()
{
    std::string grp;
    if (std::isspace(c))
    {
        while (std::isspace(c))
            consume();
    }
    while (!std::isspace(c) && curr != end && c != ';')
    {
        grp += c;
        consume();
    }
    return grp;
}

bool masm::Lexer::eof()
{
    return curr == end;
}

size_t masm::Lexer::get_col()
{
    return col;
}

size_t masm::Lexer::get_line()
{
    return line;
}

std::string masm::Lexer::get_from_line(size_t l)
{
    // using a very inefficient way
    if (l == line)
        return extract_line();
    auto temp = curr;
    size_t i = line;
    while (l != i)
    {
        if (*temp == '\n')
            i--;
        temp--;
    }
    auto tmp2 = curr;
    curr = temp;
    std::string line = extract_line();
    curr = tmp2;
    return line;
}

std::optional<masm::Token> masm::Lexer::gather_quote_string()
{
    std::string str;
    size_t st = col, l = line;
    // This function is called when a "\"" comes across
    consume(); // get rid of it
    // The things inside the quote will last until the next line too
    while (curr != end && c != '\"')
    {
        str += c;
        consume();
    }
    if (c != '\"')
    {
        // The only thing that can happen is we reached EOF
        err(*filepath.get(), l, st, st + 1, _lexing, syntaxerr, ERR_STR, "This quote was never terminated.", get_from_line(l));
        return {};
    }
    if (str.empty())
        str += " ";
    consume();
    Token t;
    t.col = st;
    t.line = l;
    t.type = STR;
    t.val = str;
    return t;
}

void masm::Lexer::set_flag_ignore_dots(bool val)
{
    ignore_dots = val;
}