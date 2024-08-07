#include "lexer.hpp"

masm::Lexer::Lexer(std::string filename)
{
    path = (std::filesystem::current_path() / filename).string();
}

bool masm::Lexer::validate_file()
{
    if (!std::filesystem::exists(std::filesystem::path(path)))
    {
        log("File Error: The file " + path + " doesn't exist.");
        return false;
    }
    return true;
}

bool masm::Lexer::validate_file(std::string name)
{
    path = (std::filesystem::current_path() / name).string();
    if (!std::filesystem::exists(std::filesystem::path(path)))
    {
        log("File Error: The file " + path + " doesn't exist.");
        return false;
    }
    return true;
}

bool masm::Lexer::init_lexer()
{
    std::fstream in(path, std::ios::in);
    if (!in.is_open())
    {
        interr("Couldn't open the given file " + path);
        return false;
    }
    while (!in.eof())
    {
        std::string line;
        std::getline(in, line);
        fileconts += line + '\n';
    }
    iter = fileconts.begin();
    in.close();
    return true;
}

masm::Token masm::Lexer::next_token()
{
    Token t;
    t.type = TOK_EOF; // by default
    size_t st;
    while (true)
    {
        if (*iter == '\0')
        {
            t.type = TOK_EOF;
            break;
        }
        else if (*iter == ';')
        {
            if (*(iter + 1) != ';')
            {
                st = col;
                iter++;
                elog(path, line, st, col, "FATAL", "Comments need ';;' but you have only ';' here.", extract_current_line());
                t.type = TOK_ERR;
                break;
            }
            else
            {
                while (*iter != '\n' && *iter != '\0')
                {
                    iter++;
                    offset++;
                }
            }
        }
        else if (std::isspace(*iter))
        {
            while (std::isspace(*iter))
            {
                if (*iter == '\n')
                {
                    col = 0;
                    line++;
                }
                else
                    col++;
                iter++;
                offset++;
            }
        }
        else if (std::isdigit(*iter))
        {
            int dot_count = 0;
            st = col;
            while (std::isdigit(*iter) || (*iter == '.' && !ignore_dot))
            {
                if (*iter == '.')
                    dot_count++;
                if (dot_count > 1)
                {
                    elog(path, line, st, col, "FATAL", "A floating point number cannot have more than one points.", extract_current_line());
                    t.type = TOK_ERR;
                    break;
                }
                t.val += *iter;
                offset++;
                col++;
                iter++;
            }
            if (t.type == TOK_ERR)
                break;
            if (dot_count == 1)
                t.type = TOK_FLOAT;
            else
                t.type = TOK_NUM;
            break;
        }
        else if (std::isalpha(*iter) || *iter == '_')
        {
            st = col;
            while (std::isalpha(*iter) || *iter == '_')
            {
                t.val += *iter;
                offset++;
                col++;
                iter++;
            }
            auto iskey = keymap.find(t.val);
            if (iskey == keymap.end())
                t.type = TOK_ID;
            else
                t.type = iskey->second;
            break;
        }
        else if (*iter == ':')
        {
            iter++;
            col++;
            offset++;
            t.type = TOK_SEMI;
            break;
        }
        else if (*iter == '\"')
        {
            // we don't accept the single quotes
            auto ret = get_string();
            if (!ret.has_value())
                t.type = TOK_ERR;
            else
            {
                t.type = TOK_BARRAY;
                t.val = ret.value();
            }
            break;
        }
        else if (*iter == ',')
        {
            iter++;
            col++;
            offset++;
        }
        else
        {
            if ((*iter == '.' && ignore_dot))
            {
                iter++;
                col++;
                offset++;
                continue;
            }
            elog(path, line, st, col, "FATAL", "Cannot build a token with this.", extract_current_line());
            t.type = TOK_ERR;
            break;
        }
    }
    t.loc.col_st = st;
    t.loc.line = line;
    t.loc.offset = offset;
    return t;
}

std::string masm::Lexer::extract_current_line()
{
    auto st = iter;
    auto ed = iter;

    while (st != fileconts.begin() && *(st - 1) != '\n')
        st--;

    while (ed != fileconts.end() && *(ed + 1) != '\n')
        ed++;

    return std::string(st, ed);
}

std::string masm::Lexer::extract_just_text()
{
    std::string text;
    while (std::isspace(*iter))
    {
        if (*iter == '\n')
        {
            col = 0;
            line++;
        }
        else
            col++;
        iter++;
        offset++;
    }
    while (!std::isspace(*iter) && *iter != '\0')
    {
        text += *iter;
        iter++;
        col++;
        offset++;
    }
    return text;
}

size_t masm::Lexer::get_line()
{
    return line;
}

size_t masm::Lexer::get_col()
{
    return col;
}

size_t masm::Lexer::get_off()
{
    return offset;
}

std::optional<std::string> masm::Lexer::get_string()
{
    size_t st = col;
    iter++;
    col++;
    offset++;
    std::string str;
    while (*iter != '\"')
    {
        if (*iter == '\0')
        {
            elog(path, line, st, col, "FATAL", "The string was never terminated; reached EOF.", extract_current_line());
            return {};
        }
        if (*iter == '\n')
        {
            elog(path, line, st, col, "FATAL", "The string was never terminated; reached EOL.", extract_current_line());
            return {};
        }
        str += *iter;
        iter++;
        col++;
        offset++;
    }
    st = col;
    iter++;
    col++;
    offset++;
    return std::make_optional<std::string>(str);
}

std::optional<std::vector<masm::Token>> masm::Lexer::get_str()
{
    std::vector<Token> str;
    ignore_dot = true;
    Token r = next_token();
    while (r.type != TOK_EOF && r.type != TOK_ERR && !std::isspace(*iter))
    {
        str.push_back(r);
        r = next_token();
    }
    ignore_dot = false;
    return std::make_optional<std::vector<Token>>(str);
}