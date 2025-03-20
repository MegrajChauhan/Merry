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
    if (_curr == '\n')
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
        if (_curr != until)
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

std::optional<masm::Token> masm::Lexer::next_token(bool _is_expr)
{
    Token t;
    t.type = MB_C;
    while (curr != end)
    {
        l = line;
        if (_curr == ';')
        {
            c = col;
            if (peek() == ';')
            {
                // a comment, so get rid of it
                rid_until('\n');
                consume();
            }
            else
            {
                log(*filepath.get(), "Invalid use of comments: Use ';;'", l, c, line, col, get_from_line(l, line));
                return {};
            }
        }
        else if (std::isspace(_curr))
        {
            while (std::isspace(_curr))
                consume();
        }
        else if (std::isalpha(_curr) || _curr == '_' || _curr == ':' || _curr == '@')
        {
            c = col;
            t = group_word();
            break;
        }
        else if (std::isdigit(_curr) || (_curr == '-' && std::isdigit(peek())))
        {
            c = col;
            auto res = group_num();
            if (!res.has_value())
                return {};
            t = res.value();
            break;
        }
        else if (_curr == '-')
        {
            // if the above condition didn't work then this is a sole minus sign
            c = col;
            t.type = OPER_MINUS;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '+')
        {
            c = col;
            t.type = OPER_PLUS;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '*')
        {
            c = col;
            t.type = OPER_MUL;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '/')
        {
            c = col;
            t.type = OPER_DIV;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '(')
        {
            c = col;
            t.type = OPER_OPEN_PAREN;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == ')')
        {
            c = col;
            t.type = OPER_CLOSE_PAREN;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '&')
        {
            c = col;
            t.type = OPER_LAND;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '|')
        {
            c = col;
            t.type = OPER_LOR;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '!')
        {
            c = col;
            t.type = OPER_NOT;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '<')
        {
            c = col;
            t.type = OPER_LS;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '>')
        {
            c = col;
            t.type = OPER_RS;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '~')
        {
            c = col;
            t.type = OPER_LNOT;
            t.col = c;
            t.line = line;
            consume();
            break;
        }
        else if (_curr == '\"')
        {
            c = col;
            auto res = gather_quote_string();
            if (!res.has_value())
                return {};
            t = res.value();
            break;
        }
        else if (_curr == ',')
        {
            consume();
        }
        else if (_curr == '[')
        {
            c = col;
            consume();
            while (curr != end && _curr != ']')
            {
                if (_curr == '[' && _is_expr)
                {
                    log(*filepath.get(), "Nested expressions are not a thing I am afraid.", l, c, line, col, get_from_line(l, line));
                    return {};
                }
                auto _t = next_token();
                if (!_t.has_value())
                {
                    log(*filepath.get(), "Expected an identifier, constant or an operator.", l, c, line, col, get_from_line(l, line));
                    return {};
                }
                t.expr.push_back(_t.value());
            }
            if (curr == end && *(curr - 1) != ']')
            {
                log(*filepath.get(), "Expression must end with a ']' but reached EOF.", l, c, line, col, get_from_line(l, line));
                return {};
            }
            t.line = l;
            t.type = EXPR;
            t.col = col;
            t.le = line;
            consume();
            break;
        }
        else
        {
            c = col;
            if (_curr == '.' && ignore_dots)
            {
                t.col = col;
                t.line = line;
                t.type = OPER_DOT;
                consume();
                break;
            }
            else
            {
                log(*filepath.get(), "Cannot build a token from this.", l, c, line, col, get_from_line(l, line));
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
    while (std::isalnum(_curr) || _curr == '_' || _curr == ':' || _curr == '@')
    {
        wrd += _curr;
        consume();
    }
    auto res = iden_map.find(wrd);
    Token t;
    t.col = c;
    t.line = l;
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
    if (_curr == '-')
    {
        val += _curr;
        consume();
    }
    char _c = peek();
    if (_curr == '0' && (_c == 'x' || _c == 'b' || _c == 'o'))
    {
        // These are not normal numbers
        // We will do something that shouldn't be done in real assemblers intended for commercial purposes
        // We will read these numbers, convert to integers and convert back to string
        // This way we avoid changing a lot of the code
        consume();
        switch (_c)
        {
        case 'x':
            val = read_hex(val.starts_with('-'));
            break;
        case 'o':
            val = read_oct(val.starts_with('-'));
            break;
        case 'b':
            val = read_bin(val.starts_with('-'));
            break;
        }
        t.type = NUM_INT;
    }
    else
    {
        while (std::isdigit(_curr) || _curr == '.')
        {
            if (_curr == '.')
            {
                if (ignore_dots)
                    break;
                d_count++;
            }
            if (d_count > 1)
            {
                log(*filepath.get(), "More than one '.' are not allowed.", l, c, line, col, get_from_line(l, line));
                return {};
            }
            val += _curr;
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
    }
    t.col = c;
    t.line = l;
    t.val = val;
    return std::make_optional<Token>(t);
}

std::string masm::Lexer::get_a_group()
{
    std::string grp;
    if (std::isspace(_curr))
    {
        while (std::isspace(_curr))
            consume();
    }
    while (!std::isspace(_curr) && curr != end && _curr != ';')
    {
        grp += _curr;
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

size_t masm::Lexer::get_col_st()
{
    return c;
}

size_t masm::Lexer::get_line()
{
    return line;
}

size_t masm::Lexer::get_line_st()
{
    return l;
}

std::string masm::Lexer::get_from_line(size_t l, size_t e)
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
    if (e != 0 && e != l)
    {
        while (i != e)
        {
            if (*temp == '\n')
                i++;
            temp++;
        }
        tmp2 = curr;
        curr = temp;
        line += extract_line();
        curr = tmp2;
    }
    return line;
}

std::optional<masm::Token> masm::Lexer::gather_quote_string()
{
    std::string str;
    // This function is called when a "\"" comes across
    consume(); // get rid of it
    // The things inside the quote will last until the next line too
    while (curr != end && _curr != '\"')
    {
        str += _curr;
        consume();
    }
    if (_curr != '\"')
    {
        // The only thing that can happen is we reached EOF
        log(*filepath.get(), "This quote was never terminated.", l, c, line, col, get_from_line(l, line));
        return {};
    }
    if (str.empty())
        str += " ";
    consume();
    Token t;
    t.col = c;
    t.line = l;
    t.type = STR;
    t.val = str;
    return t;
}

void masm::Lexer::set_flag_ignore_dots(bool val)
{
    ignore_dots = val;
}

std::string masm::Lexer::read_hex(bool neg)
{
    std::string h = neg ? "-0x" : "0x";
    consume();
    if (!(std::isdigit(_curr) || (_curr >= 'A' && _curr <= 'F')))
    {
        log(*filepath.get(), "Invalid Hexadecimal value: Where are the numbers?", l, c, line, col, get_from_line(l, line));
        exit(1);
    }
    while (std::isdigit(_curr) || (_curr >= 'A' && _curr <= 'F'))
    {
        h += _curr;
        consume();
    }
    uint64_t tmp = std::stoull(h, NULL, 16);
    h = std::to_string(tmp);
    return h;
}

std::string masm::Lexer::read_oct(bool neg)
{
    std::string h = neg ? "-0" : "0";
    consume();
    if (!(_curr >= '0' && _curr <= '7'))
    {
        log(*filepath.get(), "Invalid Octal value: Where are the numbers?", l, c, line, col, get_from_line(l, line));
        exit(1);
    }
    while ((_curr >= '0' && _curr <= '7'))
    {
        h += _curr;
        consume();
    }
    uint64_t tmp = std::stoull(h, NULL, 8);
    h = std::to_string(tmp);
    return h;
}

std::string masm::Lexer::read_bin(bool neg)
{
    std::string h = neg ? "-0b" : "0b";
    consume();
    if (!(_curr == '0' || _curr == '1'))
    {
        log(*filepath.get(), "Invalid Binary value: Where are the numbers?", l, c, line, col, get_from_line(l, line));
        exit(1);
    }
    while ((_curr == '0' || _curr == '1'))
    {
        h += _curr;
        consume();
    }
    uint64_t tmp = std::stoull(h, NULL, 2);
    h = std::to_string(tmp);
    return h;
}