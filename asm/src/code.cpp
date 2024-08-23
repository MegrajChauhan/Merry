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
            if (!handle_names(false))
                return false;
            break;
        case KEY_PROC:
            if (!handle_names(true))
                return false;
            break;
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
        case INST_RET:
        {
            Node n;
            n.kind = RET;
            n.node = std::make_unique<Base>();
            nodes->push_back(std::move(n));
            break;
        }
        case INST_ADD:
            if (!handle_arithmetic_unsigned(ADD_IMM))
                return false;
            break;
        case INST_SUB:
            if (!handle_arithmetic_unsigned(SUB_IMM))
                return false;
            break;
        case INST_MUL:
            if (!handle_arithmetic_unsigned(MUL_IMM))
                return false;
            break;
        case INST_DIV:
            if (!handle_arithmetic_unsigned(DIV_IMM))
                return false;
            break;
        case INST_MOD:
            if (!handle_arithmetic_unsigned(MOD_IMM))
                return false;
            break;
        case INST_IADD:
            if (!handle_arithmetic_signed(IADD_IMM))
                return false;
            break;
        case INST_ISUB:
            if (!handle_arithmetic_signed(ISUB_IMM))
                return false;
            break;
        case INST_IMUL:
            if (!handle_arithmetic_signed(IMUL_IMM))
                return false;
            break;
        case INST_IDIV:
            if (!handle_arithmetic_signed(IDIV_IMM))
                return false;
            break;
        case INST_IMOD:
            if (!handle_arithmetic_signed(IMOD_IMM))
                return false;
            break;
        case INST_ADDF:
            if (!handle_arithmetic_float(FADD))
                return false;
            break;
        case INST_ADDLF:
            if (!handle_arithmetic_float(LFADD))
                return false;
            break;
        case INST_SUBF:
            if (!handle_arithmetic_float(FSUB))
                return false;
            break;
        case INST_SUBLF:
            if (!handle_arithmetic_float(LFSUB))
                return false;
            break;
        case INST_MULF:
            if (!handle_arithmetic_float(FMUL))
                return false;
            break;
        case INST_MULLF:
            if (!handle_arithmetic_float(LFMUL))
                return false;
            break;
        case INST_DIVF:
            if (!handle_arithmetic_float(FDIV))
                return false;
            break;
        case INST_DIVLF:
            if (!handle_arithmetic_float(LFDIV))
                return false;
            break;
        case INST_MOV:
            if (!handle_mov(MOV_IMM))
                return false;
            break;
        case INST_MOVL:
            if (!handle_mov(MOVL_IMM))
                return false;
            break;
        case INST_MOVB:
            if (!handle_movX(MOVB))
                return false;
            break;
        case INST_MOVW:
            if (!handle_movX(MOVW))
                return false;
            break;
        case INST_MOVD:
            if (!handle_movX(MOVD))
                return false;
            break;
        case INST_MOVESXB:
            if (!handle_mov(MOVSXB_IMM))
                return false;
            break;
        case INST_MOVESXW:
            if (!handle_mov(MOVSXW_IMM))
                return false;
            break;
        case INST_MOVESXD:
            if (!handle_mov(MOVSXD_IMM))
                return false;
            break;
        case INST_JMP:
            if (!handle_jmp(JMP))
                return false;
            break;
        case INST_CALL:
            if (!handle_call())
                return false;
            break;
        case INST_SVA:
            if (!handle_sva_svc(SVA_IMM))
                return false;
            break;
        case INST_SVC:
            if (!handle_sva_svc(SVC_IMM))
                return false;
            break;
        case INST_PUSHA:
        {
            Node n;
            n.node = std::make_unique<Base>();
            n.kind = PUSHA;
            nodes->push_back(std::move(n));
            break;
        }
        case INST_POPA:
        {
            Node n;
            n.node = std::make_unique<Base>();
            n.kind = POPA;
            nodes->push_back(std::move(n));
            break;
        }
        case INST_PUSH:
            if (!handle_push_pop(PUSH_IMM))
                return false;
            break;
        case INST_POP:
            if (!handle_push_pop(POP_IMM))
                return false;
            break;
        case INST_NOT:
            if (!handle_single_regr(NOT))
                return false;
            break;
        case INST_INC:
            if (!handle_single_regr(INC))
                return false;
            break;
        case INST_DEC:
            if (!handle_single_regr(DEC))
                return false;
            break;
        case INST_AND:
            if (!handle_logical_inst(AND_IMM, false))
                return false;
            break;
        case INST_OR:
            if (!handle_logical_inst(OR_IMM, false))
                return false;
            break;
        case INST_XOR:
            if (!handle_logical_inst(XOR_IMM, false))
                return false;
            break;
        case INST_LSHIFT:
            if (!handle_logical_inst(LSHIFT, false))
                return false;
            break;
        case INST_RSHIFT:
            if (!handle_logical_inst(RSHIFT, false))
                return false;
            break;
        case INST_CMP:
            if (!handle_cmp())
                return false;
            break;
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
    node.kind = (_proc) ? PROC_DECLR : LABEL;
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
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(_l.get_line()));
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
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_mov(NodeKind k)
{
    Node node;
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
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(_l.get_line()));
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        if (check_var(t.val) && check_lbl(t.val))
        {
            err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "The identifier \"" + t.val + "\" doesn't exist.", _l.get_from_line(t.line));
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
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_sva_svc(NodeKind k)
{
    Node node;

    node.node = std::make_unique<NodeSTACK>();
    NodeSTACK *a = (NodeSTACK *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the stack instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the stack instruction.", _l.get_from_line(t.line));
        return false;
    }
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(_l.get_line()));
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
    default:
    {
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            node.kind = (NodeKind)(k + 1);
            a->second_oper = regr_map.find(t.type)->second;
            break;
        }
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_movX(NodeKind k)
{
    Node node;
    node.kind = k;
    node.node = std::make_unique<NodeMov>();

    // we don't really need info for this node and so we ignore the columns and such metadata
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
    // now for the second register operand
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register here after the first operand.", _l.get_from_line(_l.get_line()));
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
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    nodes->push_back(std::move(node));
    return true; // that's it!
}

bool masm::Code::handle_arithmetic_signed(NodeKind k)
{
    Node node;

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

bool masm::Code::handle_jmp(NodeKind k)
{
    Node node;
    node.kind = k;
    node.node = std::make_unique<NodeName>();
    NodeName *n = (NodeName *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label here after the branch instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (t.type != IDENTIFIER)
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label here after the branch instruction.", _l.get_from_line(regr_line));
        return false;
    }
    if (label_list->find(t.val) == label_list->end())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label here that exists after the branch instruction; Label doesn't exists.", _l.get_from_line(regr_line));
        return false;
    }

    n->name = t.val;
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_call()
{
    Node node;
    node.node = std::make_unique<NodeCall>();
    NodeCall *n = (NodeCall *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label here or a register after the branch instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (t.type != IDENTIFIER && !(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label here or a register after the branch instruction.", _l.get_from_line(regr_line));
        return false;
    }
    if (t.type == IDENTIFIER && (label_list->find(t.val) == label_list->end()))
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label here that exists after the branch instruction; Label doesn't exists.", _l.get_from_line(regr_line));
        return false;
    }
    node.kind = (t.type == IDENTIFIER) ? CALL : CALL_REG;

    if (t.type == IDENTIFIER)
        n->_oper = t.val;
    else
        n->_oper = regr_map[(t.type)];
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::check_var(std::string var)
{
    return table->_var_list.find(var) == table->_var_list.end();
}

bool masm::Code::check_lbl(std::string var)
{
    return (proc_list->find(var) == proc_list->end());
}

bool masm::Code::handle_push_pop(NodeKind k)
{
    Node node;
    node.node = std::make_unique<NodePushPop>();
    auto n = (NodePushPop *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label, or a register, or an identifier or an immediate after the STACK instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        if (check_lbl(t.val) && check_var(t.val))
        {
            err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label or an identifier here that exists after the STACK instruction.", _l.get_from_line(regr_line));
            return false;
        }
        node.kind = (NodeKind)(k + 2);
        n->val = t.val;
        break;
    }
    case NUM_FLOAT:
    case NUM_INT:
    {
        if (k == POP_IMM)
        {
            err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "POP instruction doesn't accept immediates.", _l.get_from_line(regr_line));
            return false;
        }
        node.kind = (k);
        n->val = t.val;
        break;
    }
    default:
        if (t.type >= KEY_Ma && t.type <= KEY_Mm5)
        {
            node.kind = (NodeKind)(k + 1);
            n->val = regr_map[t.type];
            break;
        }
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a label, identifier,or an immediate here that after the STACK instruction.", _l.get_from_line(regr_line));
        return false;
    }

    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_single_regr(NodeKind k)
{
    Node node;
    node.node = std::make_unique<NodeSingleRegr>();
    auto n = (NodeSingleRegr *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here as the operand.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here as the operand.", _l.get_from_line(regr_line));
        return false;
    }
    n->reg = regr_map[t.type];
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_logical_inst(NodeKind k, bool limit)
{
    Node node;
    node.node = std::make_unique<NodeLogical>();
    auto n = (NodeLogical *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register or an immediate after the LOGICAL instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    n->reg = regr_map[t.type];
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected an immediate or another register after the LOGICAL instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case NUM_INT:
    {
        node.kind = (k);
        n->second_oper = t.val;
        break;
    }
    default:
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            if (limit)
            {
                err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "This logical instruction doesn't accept registers as second.", _l.get_from_line(regr_line));
                return false;
            }
            node.kind = (NodeKind)(k + 1);
            n->second_oper = regr_map[t.type];
            break;
        }
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected an immediate or a register here that after the LOGICAL instruction.", _l.get_from_line(regr_line));
        return false;
    }
    nodes->push_back(std::move(node));
    return true;
}

bool masm::Code::handle_cmp()
{
    Node node;
    node.node = std::make_unique<NodeLogical>();
    auto a = (NodeLogical *)node.node.get();
    auto res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, regr_line, regr_col, regr_col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the CMP instruction.", _l.get_from_line(regr_line));
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register here after the CMP instruction.", _l.get_from_line(t.line));
        return false;
    }
    res = _l.next_token();
    if (!res.has_value())
    {
        err(fname, _l.get_line(), _l.get_col(), 0, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(_l.get_line()));
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
            err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "The identifier \"" + t.val + "\" doesn't exist.", _l.get_from_line(t.line));
            return false;
        }
        node.kind = CMP_VAR;
        a->second_oper = t.val;
        break;
    }
    case NUM_INT:
    {
        node.kind = CMP_IMM;
        a->second_oper = t.val;
        break;
    }
    default:
    {
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            node.kind = CMP_REG;
            a->second_oper = regr_map.find(t.type)->second;
            break;
        }
        err(fname, t.line, t.col, t.col + 1, _parsing, syntaxerr, ERR_STR, "Expected a register, variable or immediate here after the first operand.", _l.get_from_line(t.line));
        return false;
    }
    }
    nodes->push_back(std::move(node));
    return true;
    // Yes this entire function is a copy of the MOV instruction
    // Get offended all you want
}
