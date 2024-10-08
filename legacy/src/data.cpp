#include "data.hpp"

void masm::Data::setup_for_read(SymbolTable *t, std::shared_ptr<std::string> f, std::shared_ptr<std::string> fconts)
{
    sym = t;
    file = f;
    fname = *f.get();
    l.setup_lexer(fconts, f);
}

bool masm::Data::read_data()
{
    while (!l.eof())
    {
        std::optional<Token> tok;
        if (read_again == true)
            tok = l.next_token();
        else
        {
            tok = old_tok;
            read_again = true;
        }
        if (!tok.has_value())
        {
            if (l.eof())
                return true;
            note("While parsing the data file \"" + fname + "\"");
            return false;
        }
        Token t = tok.value();
        switch (t.type)
        {
        case KEY_DB:
        {
            if (!handle_defines(BYTE))
                return false;
            break;
        }
        case KEY_DW:
        {
            if (!handle_defines(WORD))
                return false;
            break;
        }
        case KEY_DD:
        {
            if (!handle_defines(DWORD))
                return false;
            break;
        }
        case KEY_DQ:
        {
            if (!handle_defines(QWORD))
                return false;
            break;
        }
        case KEY_DF:
        {
            if (!handle_defines(FLOAT))
                return false;
            break;
        }
        case KEY_DLF:
        {
            if (!handle_defines(LFLOAT))
                return false;
            break;
        }
        case KEY_DC:
        {
            if (!handle_defines(BYTE, true))
                return false;
            break;
        }
        case KEY_RB:
        {
            if (!handle_defines(RESB))
                return false;
            break;
        }
        case KEY_RW:
        {
            if (!handle_defines(RESW))
                return false;
            break;
        }
        case KEY_RD:
        {
            if (!handle_defines(RESD))
                return false;
            break;
        }
        case KEY_RQ:
        {
            if (!handle_defines(RESQ))
                return false;
            break;
        }
        case KEY_DS:
        {
            if (!handle_strings())
                return false;
            break;
        }
        default:
            err(fname, t.line, t.col, t.val.length(), _parsing, straytok, ERR_STR, "A stray token that doesn't fit any rules was found.", l.get_from_line(t.line), "Maybe a fluke? Forgot a keyword?");
            return false;
        }
    }
    return true;
}

bool masm::Data::handle_defines(DataType t, bool _const)
{
    Token id, value;
    bool has_value = true;
    Variable var;
    auto res = l.next_token();
    if (!res.has_value())
    {
        err(fname, l.get_line(), l.get_col(), l.get_col() + 1, _parsing, syntaxerr, ERR_STR, "Expected an identifier after a definition key.", l.extract_line());
        return false;
    }
    id = res.value();
    if (id.type != IDENTIFIER)
    {
        err(fname, id.line, id.col, l.get_col(), _parsing, syntaxerr, ERR_STR, "Expected an identifier after a definition key.", l.extract_line());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
        has_value = false;
    else
    {
        value = res.value();
        // Yes, db, dw etc can also have floats but they are ignored in the final conversion
        if (value.type == EXPR)
        {
            var.is_expr = true;
        }
        else if (value.type != NUM_INT && value.type != NUM_FLOAT)
        {
            has_value = false;
            read_again = false;
            old_tok = value;
        }
    }
    var.file = file;
    var.line = id.line;
    var.name = id.val;
    var.type = (_const ? (value.type == NUM_FLOAT ? FLOAT : BYTE) : t);
    var.value = has_value ? value.val : "0";
    var.expr = value.expr;
    return _const ? add_const(var) : add_variable(var);
}

bool masm::Data::add_variable(Variable v)
{
    // if the variable already exists, throw error
    auto res = sym->_var_list.find(v.name);
    if (res != sym->_var_list.end())
    {
        // redefinition
        Variable tmp = sym->variables[res->second];
        ld_err(fname, v.line, _parsing, redefin, ERR_STR, "Redefinition of variable \"" + v.name + "\"; In line " + std::to_string(tmp.line), l.get_from_line(v.line));
        fu_err(*tmp.file.get(), tmp.line, "Defined here firstly as a variable.");
        return false;
    }
    sym->variables.push_back(v);
    sym->_var_list[v.name] = sym->variables.size() - 1;
    return true;
}

bool masm::Data::add_const(Variable v)
{
    auto res = sym->_const_list.find(v.name);
    if (res != sym->_const_list.end())
    {
        Variable tmp = res->second;
        ld_err(fname, v.line, _parsing, redefin, ERR_STR, "Redefinition of CONSTANT \"" + v.name + "\"; In line " + std::to_string(tmp.line), l.get_from_line(v.line));
        fu_err(*tmp.file.get(), tmp.line, "Defined here firstly as a CONSTANT.");
        return false;
    }
    sym->_const_list[v.name] = v;
    return true;
}

bool masm::Data::handle_strings()
{
    Token id;
    auto res = l.next_token();
    if (!res.has_value())
    {
        err(fname, l.get_line(), l.get_col(), l.get_col() + 1, _parsing, syntaxerr, ERR_STR, "Expected an identifier after a definition key.", l.extract_line());
        return false;
    }
    id = res.value();
    if (id.type != IDENTIFIER)
    {
        err(fname, id.line, id.col, l.get_col(), _parsing, syntaxerr, ERR_STR, "Expected an identifier after a definition key.", l.extract_line());
        return false;
    }
    l.set_flag_ignore_dots(true);
    Token _t;
    res = l.next_token();
    if (!res.has_value())
    {
        err(fname, id.line, id.col, id.val.length(), _parsing, syntaxerr, ERR_STR, "A string cannot have a default value.", l.get_from_line(id.line));
        return false;
    }
    _t = res.value();
    std::string final_val;
    if (_t.type == OPER_DOT)
    {
        err(fname, _t.line, _t.col, 0, _parsing, syntaxerr, ERR_STR, "The strings should not start with a '.'", l.get_from_line(_t.line));
        return false;
    }
    final_val += _t.val;
    res = l.next_token();
    while (res.has_value() && (_t = res.value()).type == OPER_DOT)
    {
        res = l.next_token();
        if (!res.has_value())
        {
            err(fname, l.get_line(), l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a value or a string after the '.' operator.", l.extract_line());
            return false;
        }
        _t = res.value();
        switch (_t.type)
        {
        case STR:
        {
            final_val += _t.val;
            break;
        }
        case NUM_INT:
        {
            final_val += (char)(std::stoi(_t.val) & 255);
            break;
        }
        default:
            err(fname, _t.line, _t.col, 0, _parsing, syntaxerr, ERR_STR, "There can only be either integers or strings after the '.' operator.", l.get_from_line(_t.line));
            return false;
        }
        res = l.next_token();
    }
    if (_t.type != OPER_DOT)
    {
        read_again = false;
        old_tok = _t;
    }
    Variable v;
    v.file = file;
    v.line = id.line;
    v.name = id.val;
    v.type = STRING;
    v.value = final_val;
    l.set_flag_ignore_dots(false);
    return add_variable(v);
}