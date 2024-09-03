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

std::optional<masm::Token> masm::Lexer::next_token(bool _is_expr)
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
        else if (std::isalpha(c) || c == '_' || c == ':' || c == '@')
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
        else if (c == '-')
        {
            // if the above condition didn't work then this is a sole minus sign
            t.type = OPER_MINUS;
            break;
        }
        else if (c == '+')
        {
            t.type = OPER_PLUS;
            break;
        }
        else if (c == '*')
        {
            t.type = OPER_MUL;
            break;
        }
        else if (c == '/')
        {
            t.type = OPER_DIV;
            break;
        }
        else if (c == '(')
        {
            t.type = OPER_OPEN_PAREN;
            break;
        }
        else if (c == ')')
        {
            t.type = OPER_CLOSE_PAREN;
            break;
        }
        else if (c == '&')
        {
            t.type = OPER_LAND;
            break;
        }
        else if (c == '|')
        {
            t.type = OPER_LOR;
            break;
        }
        else if (c == '!')
        {
            t.type = OPER_NOT;
            break;
        }
        else if (c == '<')
        {
            t.type = OPER_LS;
            break;
        }
        else if (c == '>')
        {
            t.type = OPER_RS;
            break;
        }
        else if (c == '~')
        {
            t.type = OPER_LNOT;
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
        else if (c == '[')
        {
            t.col = col;
            t.line = line;
            consume();
            while (curr != end && c != ']')
            {
                if (c == '[' && _is_expr)
                {
                    err(*filepath.get(), line, col, col + 1, _lexing, syntaxerr, ERR_STR, "Nested expressions aren't a thing here.", extract_line());
                    return {};
                }
                auto _t = next_token();
                if (!_t.has_value())
                {
                    err(*filepath.get(), line, col, col + 1, _lexing, syntaxerr, ERR_STR, "Expected an identifier, constant or an operator.", extract_line());
                    return {};
                }
                t.expr.push_back(_t.value());
            }
            if (curr == end && *(curr - 1) != ']')
            {
                err(*filepath.get(), line, col, col + 1, _lexing, syntaxerr, ERR_STR, "Expression must end with a ']' but reached EOF.", extract_line());
                return {};
            }
            t.type = EXPR;
            consume();
            break;
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
    while (std::isalnum(c) || c == '_' || c == ':' || c == '@')
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
    char _c = peek();
    if (std::isdigit(c) && c == '0' && (_c == 'x' || _c == 'b' || _c == 'o'))
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
    }
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

std::string masm::Lexer::read_hex(bool neg)
{
    std::string h = neg ? "-0x" : "0x";
    consume();
    if (!(std::isdigit(c) || (c >= 'A' && c <= 'F')))
    {
        note("Invalid Hexadecimal value: Where are the numbers?");
        die(1);
    }
    while (std::isdigit(c) || (c >= 'A' && c <= 'F'))
    {
        h += c;
        consume();
    }
    long long tmp = std::stoll(h, NULL, 16);
    h = std::to_string(tmp);
    return h;
}

std::string masm::Lexer::read_oct(bool neg)
{
    std::string h = neg ? "-0o" : "0o";
    consume();
    if (!(c >= '0' && c <= '7'))
    {
        note("Invalid Octal value: Where are the numbers?");
        die(1);
    }
    while ((c >= '0' && c <= '7'))
    {
        h += c;
        consume();
    }
    long long tmp = std::stoll(h, NULL, 8);
    h = std::to_string(tmp);
    return h;
}

std::string masm::Lexer::read_bin(bool neg)
{
    std::string h = neg ? "-0b" : "0b";
    consume();
    if (!(c == '0' || c == '1'))
    {
        note("Invalid Binary value: Where are the numbers?");
        die(1);
    }
    while ((c == '0' || c == '1'))
    {
        h += c;
        consume();
    }
    long long tmp = std::stoll(h, NULL, 2);
    h = std::to_string(tmp);
    return h;
}