#include "code.hpp"

void masm::Code::setup_code_read(std::vector<Node> *n, std::unordered_map<std::string, Procedure> *pl, std::shared_ptr<std::string> fn, std::shared_ptr<std::string> fcont, SymbolTable *sym, std::unordered_map<std::string, size_t> *lbl_list)
{
    nodes = n;
    proc_list = pl;
    file = fn;
    fname = *fn.get();
    _l.setup_lexer(fcont, fn);
    table = sym;
    label_list = lbl_list;
}

bool masm::Code::read_code()
{
    std::optional<Token> res;
    while (!_l.eof())
    {
        res = _l.next_token();
        if (!res.has_value())
            if (!_l.eof())
                return false;
            else
                return true;
        t = res.value();
        regr;
        switch (t.type)
        {
        case IDENTIFIER:
        {
            if (!handle_names(false))
                return false;
            break;
        }
        case KEY_PROC:
        {
            if (!handle_names(true))
                return false;
            break;
        }
        case INST_HLT:
        {
            Node n;
            n.kind = HLT;
            n.node = std::make_unique<Base>();
            nodes->push_back(std::move(n));
            break;
        }
        case INST_NOP:
        {
            Node n;
            n.kind = NOP;
            n.node = std::make_unique<Base>();
            nodes->push_back(std::move(n));
            break;
        }
        case INST_ADD:
        {
            if (!handle_arithmetic_unsigned(ADD_IMM))
                return false;
            break;
        }
        case INST_SUB:
        {
            if (!handle_arithmetic_unsigned(SUB_IMM))
                return false;
            break;
        }
        case INST_MUL:
        {
            if (!handle_arithmetic_unsigned(MUL_IMM))
                return false;
            break;
        }
        case INST_DIV:
        {
            if (!handle_arithmetic_unsigned(DIV_IMM))
                return false;
            break;
        }
        case INST_MOD:
        {
            if (!handle_arithmetic_unsigned(MOD_IMM))
                return false;
            break;
        }
        case INST_IADD:
        {
            if (!handle_arithmetic_signed(IADD_IMM))
                return false;
            break;
        }
        case INST_ISUB:
        {
            if (!handle_arithmetic_signed(ISUB_IMM))
                return false;
            break;
        }
        case INST_IMUL:
        {
            if (!handle_arithmetic_signed(IMUL_IMM))
                return false;
            break;
        }
        case INST_IDIV:
        {
            if (!handle_arithmetic_signed(IDIV_IMM))
                return false;
            break;
        }
        case INST_IMOD:
        {
            if (!handle_arithmetic_signed(IMOD_IMM))
                return false;
            break;
        }
        case INST_ADDF:
        {
            if (!handle_arithmetic_float(FADD))
                return false;
            break;
        }
        case INST_ADDLF:
        {
            if (!handle_arithmetic_float(LFADD))
                return false;
            break;
        }
        case INST_SUBF:
        {
            if (!handle_arithmetic_float(FSUB))
                return false;
            break;
        }
        case INST_SUBLF:
        {
            if (!handle_arithmetic_float(LFSUB))
                return false;
            break;
        }
        case INST_MULF:
        {
            if (!handle_arithmetic_float(FMUL))
                return false;
            break;
        }
        case INST_MULLF:
        {
            if (!handle_arithmetic_float(LFMUL))
                return false;
            break;
        }
        case INST_DIVF:
        {
            if (!handle_arithmetic_float(FDIV))
                return false;
            break;
        }
        case INST_DIVLF:
        {
            if (!handle_arithmetic_float(LFDIV))
                return false;
            break;
        }
        case INST_MOV:
        {
            if (!handle_mov(MOV_IMM))
                return false;
            break;
        }
        case INST_MOVL:
        {
            if (!handle_mov(MOVL_IMM))
                return false;
            break;
        }

        default:
            err(fname, t.line, t.col, t.val.length(), _parsing, straytok, ERR_STR, "A stray token that doesn't fit any rules was found.", _l.get_from_line(t.line), "Maybe a fluke? Forgot a keyword?");
            return false;
        }
    }
    return true;
}

bool masm::Code::handle_names(bool _proc)
{
    if (_proc)
    {
        auto r = _l.next_token();
        if (!r.has_value())
        {
            err(fname, t.line, t.col, t.col + 4, _parsing, syntaxerr, ERR_STR, "Missing an identifier after the 'proc' keyword.", _l.get_from_line(t.line));
            return false;
        }
        t = r.value();
        if (t.type != IDENTIFIER)
        {
            err(fname, t.line, t.col, t.col + 4, _parsing, syntaxerr, ERR_STR, "Missing an identifier after the 'proc' keyword.", _l.get_from_line(t.line));
            return false;
        }
    }
    // check if the label already exists
    auto in_pl = proc_list->find(t.val);
    auto in_label = label_list->find(t.val);
    bool pl = in_pl == proc_list->end();
    bool lbl = in_label == label_list->end();
    if (_proc)
    {
        if (!pl)
        {
            // redeclaration
            err(fname, t.line, t.col, t.val.length(), _parsing, redeclr, ERR_STR, "Redeclaration of proc \"" + t.val + "\".", _l.get_from_line(t.line));
            return false;
        }
        if (!lbl)
        {
            // The label was defined before the procedure was declared
            err(fname, t.line, t.col, t.val.length(), _parsing, noorder, ERR_STR, "Declaration of proc \"" + t.val + "\" after the definition of its label.", _l.get_from_line(t.line));
            return false;
        }
    }
    else
    {
        if (!pl)
        {
            // definition
            if (!lbl)
            {
                // redefinition
                err(fname, t.line, t.col, t.val.length(), _parsing, redefin, ERR_STR, "Redefinition of procedure label \"" + t.val + "\".", _l.get_from_line(t.line));
                return false;
            }
        }
        else
        {
            if (!lbl)
            {
                // redefinition
                err(fname, t.line, t.col, t.val.length(), _parsing, redefin, ERR_STR, "Redefinition of label \"" + t.val + "\".", _l.get_from_line(t.line));
                return false;
            }
        }
    }
    std::unique_ptr<Base> n = std::make_unique<NodeName>();
    NodeName *n_ = (NodeName *)n.get();
    n_->name = t.val;
    Node node;
    node.col_st = regr_col;
    node.file = file;
    node.kind = (_proc) ? PROC_DECLR : LABEL;
    node.line_ed = t.line;
    node.line_st = regr_line;
    node.node = std::move(n);
    nodes->push_back(std::move(node));
    if (node.kind == PROC_DECLR)
    {
        Procedure p;
        p.defined = false;
        p.ind = nodes->size() - 1;
        (*proc_list)[t.val] = p;
    }
    else
    {
        (*label_list)[t.val] = nodes->size() - 1;
        if (!pl)
            in_pl->second.defined = true;
    }
    return true;
}

bool masm::Code::handle_arithmetic_unsigned(NodeKind k)
{
    Node node;
    node.col_st = regr_col;
    node.line_st = regr_line;
    node.file = file;
    node.node = std::make_unique<NodeArithmetic>();
    NodeArithmetic *a = (NodeArithmetic *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the arithmetic instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the arithmetic instruction.", _l.get_from_line(t.line));
        return false;
    }
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or another register here after the first operand.", _l.get_from_line(_l.get_line()));
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        if (check_var(t.val))
        {
            err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "The variable \"" + t.val + "\" doesn't exist.", _l.get_from_line(t.line));
            return false;
        }
        node.kind = (NodeKind)(k + 2);
        a->second_oper = t.val;
        break;
    }
    case NUM_INT:
    {
        node.kind = k;
        a->second_oper = t.val;
        break;
    }
    default:
    {
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            node.kind = (NodeKind)(k + 1);
            a->second_oper = regr_map.find(t.type)->second;
            break;
        }
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or another register here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_mov(NodeKind k)
{
    Node node;
    node.col_st = regr_col;
    node.line_st = regr_line;
    node.file = file;
    node.node = std::make_unique<NodeMov>();
    NodeMov *a = (NodeMov *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the mov instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the mov instruction.", _l.get_from_line(t.line));
        return false;
    }
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or another register here after the first operand.", _l.get_from_line(_l.get_line()));
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        if (check_var(t.val))
        {
            err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "The variable \"" + t.val + "\" doesn't exist.", _l.get_from_line(t.line));
            return false;
        }
        node.kind = (NodeKind)(k + 2);
        a->second_oper = std::make_pair(t.val, BYTE);
        break;
    }
    case NUM_INT:
    {
        node.kind = k;
        a->second_oper = std::make_pair(t.val, BYTE);
        break;
    }
    case NUM_FLOAT:
    {
        node.kind = k;
        a->second_oper = std::make_pair(t.val, FLOAT);
        break;
    }
    default:
    {
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            node.kind = (NodeKind)(k + 1);
            a->second_oper = regr_map.find(t.type)->second;
            break;
        }
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or another register here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_arithmetic_signed(NodeKind k)
{
    Node node;
    node.col_st = regr_col;
    node.line_st = regr_line;
    node.file = file;
    node.node = std::make_unique<NodeArithmetic>();
    NodeArithmetic *a = (NodeArithmetic *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the signed arithmetic instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the signed arithmetic instruction.", _l.get_from_line(t.line));
        return false;
    }
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register or another register here after the first operand.", _l.get_from_line(_l.get_line()));
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    switch (t.type)
    {
    case NUM_INT:
    {
        node.kind = k;
        a->second_oper = t.val;
        break;
    }
    default:
    {
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            node.kind = (NodeKind)(k + 1);
            a->second_oper = regr_map.find(t.type)->second;
            break;
        }
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register or another register here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_arithmetic_float(NodeKind k)
{
    Node node;
    node.col_st = regr_col;
    node.line_st = regr_line;
    node.file = file;
    node.node = std::make_unique<NodeArithmetic>();
    NodeArithmetic *a = (NodeArithmetic *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the floating-point arithmetic instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the floating-point arithmetic instruction.", _l.get_from_line(t.line));
        return false;
    }
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register or another register here after the first operand.", _l.get_from_line(_l.get_line()));
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        node.kind = (NodeKind)(k + 1);
        a->second_oper = regr_map.find(t.type)->second;
    }
    else
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register or another register here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::check_var(std::string var)
{
    auto res = table->_var_list.find(var);
    return res == table->_var_list.end();
}