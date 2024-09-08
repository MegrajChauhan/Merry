#include "parser.hpp"

void masm::Parser::setup_parser(std::string filename)
{
    fname = filename;
    filename = std::filesystem::current_path() / filename;
    if (used_files.find(filename) != used_files.end())
        return;
    if (!filename.ends_with(".masm"))
    {
        note("The given input file is not a valid input file.");
        exit(1);
    }
    if (!std::filesystem::exists(filename))
    {
        note("The given input file " + filename + " doesn't exist.");
        exit(1);
    }
    if (std::filesystem::is_directory(filename))
    {
        note("The given input file is a directory and not a file.");
        exit(1);
    }
    std::fstream ins(filename, std::ios::in);
    while (!ins.eof())
    {
        std::string l;
        std::getline(ins, l);
        fconts += l + '\n';
    }
    l.setup_lexer(std::make_shared<std::string>(fconts), std::make_shared<std::string>(fname));
    ins.close();
    file = std::make_shared<std::string>(fname);
}

void masm::Parser::parse()
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
                return;
            note("While parsing the data file \"" + fname + "\"");
            exit(1);
        }
        Token t = tok.value();
        switch (t.type)
        {
        case KEY_DB:
            if (!handle_defines(BYTE))
                exit(1);
            break;
        case KEY_DW:
            if (!handle_defines(WORD))
                exit(1);
            break;
        case KEY_DD:
            if (!handle_defines(DWORD))
                exit(1);
            break;
        case KEY_DQ:
            if (!handle_defines(QWORD))
                exit(1);
            break;
        case KEY_DF:
            if (!handle_defines(FLOAT))
                exit(1);
            break;
        case KEY_DLF:
            if (!handle_defines(LFLOAT))
                exit(1);
            break;
        case KEY_DC:
            if (!handle_defines(BYTE, true))
                exit(1);
            break;
        case KEY_RB:
            if (!handle_defines(RESB))
                exit(1);
            break;
        case KEY_RW:
            if (!handle_defines(RESW))
                exit(1);
            break;
        case KEY_RD:
            if (!handle_defines(RESD))
                exit(1);
            break;
        case KEY_RQ:
            if (!handle_defines(RESQ))
                exit(1);
            break;
        case KEY_DS:
            if (!handle_strings())
                exit(1);
            break;
        case IDENTIFIER:
            if (!handle_names(false))
                exit(1);
            break;
        case KEY_PROC:
            if (!handle_names(true))
                exit(1);
            break;
        case INST_HLT:
            handle_one(HLT);
            break;
        case INST_NOP:
            handle_one(NOP);
            break;
        case INST_RET:
            handle_one(RET);
            break;
        case INST_CFLAGS:
            handle_one(CFLAGS);
            break;
        case INST_RESET:
            handle_one(NRESET);
            break;
        case INST_CLZ:
            handle_one(CLZ);
            break;
        case INST_CLN:
            handle_one(CLN);
            break;
        case INST_CLC:
            handle_one(CLC);
            break;
        case INST_CLO:
            handle_one(CLO);
            break;
        case INST_CALLE:
            handle_one(CALLE);
            break;
        case INST_SYSCALL:
            handle_one(SYSCALL);
            break;
        case INST_ADD:
            if (!handle_arithmetic(ADD_IMM))
                exit(1);
            break;
        case INST_SUB:
            if (!handle_arithmetic(SUB_IMM))
                exit(1);
            break;
        case INST_MUL:
            if (!handle_arithmetic(MUL_IMM))
                exit(1);
            break;
        case INST_DIV:
            if (!handle_arithmetic(DIV_IMM))
                exit(1);
            break;
        case INST_MOD:
            if (!handle_arithmetic(MOD_IMM))
                exit(1);
            break;
        case INST_IADD:
            if (!handle_arithmetic(IADD_IMM))
                exit(1);
            break;
        case INST_ISUB:
            if (!handle_arithmetic(ISUB_IMM))
                exit(1);
            break;
        case INST_IMUL:
            if (!handle_arithmetic(IMUL_IMM))
                exit(1);
            break;
        case INST_IDIV:
            if (!handle_arithmetic(IDIV_IMM))
                exit(1);
            break;
        case INST_IMOD:
            if (!handle_arithmetic(IMOD_IMM))
                exit(1);
            break;
        case INST_ADDF:
            if (!handle_arithmetic_float(FADD))
                exit(1);
            break;
        case INST_ADDLF:
            if (!handle_arithmetic_float(LFADD))
                exit(1);
            break;
        case INST_SUBF:
            if (!handle_arithmetic_float(FSUB))
                exit(1);
            break;
        case INST_SUBLF:
            if (!handle_arithmetic_float(LFSUB))
                exit(1);
            break;
        case INST_MULF:
            if (!handle_arithmetic_float(FMUL))
                exit(1);
            break;
        case INST_MULLF:
            if (!handle_arithmetic_float(LFMUL))
                exit(1);
            break;
        case INST_DIVF:
            if (!handle_arithmetic_float(FDIV))
                exit(1);
            break;
        case INST_DIVLF:
            if (!handle_arithmetic_float(LFDIV))
                exit(1);
            break;
        case INST_MOV:
            if (!handle_mov(MOV_IMM))
                exit(1);
            break;
        case INST_MOVL:
            if (!handle_mov(MOVL_IMM))
                exit(1);
            break;
        case INST_MOVB:
            if (!handle_movX(MOVB))
                exit(1);
            break;
        case INST_MOVW:
            if (!handle_movX(MOVW))
                exit(1);
            break;
        case INST_MOVD:
            if (!handle_movX(MOVD))
                exit(1);
            break;
        case INST_MOVESXB:
            if (!handle_mov(MOVSXB_IMM))
                exit(1);
            break;
        case INST_MOVESXW:
            if (!handle_mov(MOVSXW_IMM))
                exit(1);
            break;
        case INST_MOVESXD:
            if (!handle_mov(MOVSXD_IMM))
                exit(1);
            break;
        case INST_JMP:
            if (!handle_jmp(JMP))
                exit(1);
            break;
        case INST_JNZ:
            if (!handle_jmp(JNZ))
                exit(1);
            break;
        case INST_JZ:
            if (!handle_jmp(JZ))
                exit(1);
            break;
        case INST_JNE:
            if (!handle_jmp(JNE))
                exit(1);
            break;
        case INST_JE:
            if (!handle_jmp(JE))
                exit(1);
            break;
        case INST_JNC:
            if (!handle_jmp(JNC))
                exit(1);
            break;
        case INST_JC:
            if (!handle_jmp(JC))
                exit(1);
            break;
        case INST_JNO:
            if (!handle_jmp(JNO))
                exit(1);
            break;
        case INST_JO:
            if (!handle_jmp(JO))
                exit(1);
            break;
        case INST_JNN:
            if (!handle_jmp(JNN))
                exit(1);
            break;
        case INST_JN:
            if (!handle_jmp(JN))
                exit(1);
            break;
        case INST_JNG:
            if (!handle_jmp(JNG))
                exit(1);
            break;
        case INST_JG:
            if (!handle_jmp(JG))
                exit(1);
            break;
        case INST_JNS:
            if (!handle_jmp(JNS))
                exit(1);
            break;
        case INST_JS:
            if (!handle_jmp(JS))
                exit(1);
            break;
        case INST_JGE:
            if (!handle_jmp(JGE))
                exit(1);
            break;
        case INST_JSE:
            if (!handle_jmp(JSE))
                exit(1);
            break;
        case INST_LOOP:
            if (!handle_jmp(LOOP))
                exit(1);
            break;
        case INST_SETE:
            if (!handle_jmp(SETE))
                exit(1);
            break;
        case INST_CALL:
            if (!handle_call())
                exit(1);
            break;
        case INST_SVA:
            if (!handle_sva_svc(SVA_IMM))
                exit(1);
            break;
        case INST_SVC:
            if (!handle_sva_svc(SVC_IMM))
                exit(1);
            break;
        case INST_PUSHA:
            handle_one(PUSHA);
            break;
        case INST_POPA:
            handle_one(POPA);
            break;
        case INST_PUSH:
            if (!handle_push_pop(PUSH_IMM))
                exit(1);
            break;
        case INST_POP:
            if (!handle_push_pop(POP_IMM))
                exit(1);
            break;
        case INST_NOT:
            if (!handle_single_regr(NOT))
                exit(1);
            break;
        case INST_INC:
            if (!handle_single_regr(INC))
                exit(1);
            break;
        case INST_DEC:
            if (!handle_single_regr(DEC))
                exit(1);
            break;
        case INST_AND:
            if (!handle_logical_inst(AND_IMM, false))
                exit(1);
            break;
        case INST_OR:
            if (!handle_logical_inst(OR_IMM, false))
                exit(1);
            break;
        case INST_XOR:
            if (!handle_logical_inst(XOR_IMM, false))
                exit(1);
            break;
        case INST_LSHIFT:
            if (!handle_logical_inst(LSHIFT, true))
                exit(1);
            break;
        case INST_RSHIFT:
            if (!handle_logical_inst(RSHIFT, true))
                exit(1);
            break;
        case INST_CMP:
            if (!handle_cmp())
                exit(1);
            break;
        case INST_LEA:
            if (!handle_lea())
                exit(1);
            break;
        case INST_LOADB:
            if (!handle_load_store(LOADB_REG))
                exit(1);
            break;
        case INST_LOADW:
            if (!handle_load_store(LOADW_REG))
                exit(1);
            break;
        case INST_LOADD:
            if (!handle_load_store(LOADD_REG))
                exit(1);
            break;
        case INST_LOADQ:
            if (!handle_load_store(LOADQ_REG))
                exit(1);
            break;
        case INST_STOREB:
            if (!handle_load_store(STOREB_REG))
                exit(1);
            break;
        case INST_STOREW:
            if (!handle_load_store(STOREW_REG))
                exit(1);
            break;
        case INST_STORED:
            if (!handle_load_store(STORED_REG))
                exit(1);
            break;
        case INST_STOREQ:
            if (!handle_load_store(STOREQ_REG))
                exit(1);
            break;
        case KEY_ATM:
            if (!handle_atm())
                exit(1);
            break;
        case INST_CMPXCHG:
            if (!handle_cmpxchg())
                exit(1);
            break;
        case INST_OUTR:
            handle_one(OUTR);
            break;
        case INST_UOUTR:
            handle_one(UOUTR);
            break;
        case INST_CIN:
            if (!handle_single_regr(CIN))
                exit(1);
            break;
        case INST_COUT:
            if (!handle_single_regr(COUT))
                exit(1);
            break;
        case INST_IN:
            if (!handle_single_regr(IN))
                exit(1);
            break;
        case INST_INW:
            if (!handle_single_regr(INW))
                exit(1);
            break;
        case INST_IND:
            if (!handle_single_regr(IND))
                exit(1);
            break;
        case INST_INQ:
            if (!handle_single_regr(INQ))
                exit(1);
            break;
        case INST_UIN:
            if (!handle_single_regr(UIN))
                exit(1);
            break;
        case INST_UINW:
            if (!handle_single_regr(UINW))
                exit(1);
            break;
        case INST_UIND:
            if (!handle_single_regr(UIND))
                exit(1);
            break;
        case INST_UINQ:
            if (!handle_single_regr(UINQ))
                exit(1);
            break;
        case INST_OUT:
            if (!handle_single_regr(OUT))
                exit(1);
            break;
        case INST_OUTW:
            if (!handle_single_regr(OUTW))
                exit(1);
            break;
        case INST_OUTD:
            if (!handle_single_regr(OUTD))
                exit(1);
            break;
        case INST_OUTQ:
            if (!handle_single_regr(OUTQ))
                exit(1);
            break;
        case INST_UOUT:
            if (!handle_single_regr(UOUT))
                exit(1);
            break;
        case INST_UOUTW:
            if (!handle_single_regr(UOUTW))
                exit(1);
            break;
        case INST_UOUTD:
            if (!handle_single_regr(UOUTD))
                exit(1);
            break;
        case INST_UOUTQ:
            if (!handle_single_regr(UOUTQ))
                exit(1);
            break;
        case INST_INF:
            if (!handle_single_regr(INF))
                exit(1);
            break;
        case INST_INLF:
            if (!handle_single_regr(INLF))
                exit(1);
            break;
        case INST_OUTF:
            if (!handle_single_regr(OUTF))
                exit(1);
            break;
        case INST_OUTLF:
            if (!handle_single_regr(OUTLF))
                exit(1);
            break;
        case INST_SIN:
            if (!handle_sio(SIN))
                exit(1);
            break;
        case INST_SOUT:
            if (!handle_sio(SOUT))
                exit(1);
            break;
        case INST_EXCHGB:
            if (!handle_excg(EXCGB))
                exit(1);
            break;
        case INST_EXCHGW:
            if (!handle_excg(EXCGW))
                exit(1);
            break;
        case INST_EXCHGD:
            if (!handle_excg(EXCGD))
                exit(1);
            break;
        case INST_EXCHGQ:
            if (!handle_excg(EXCGQ))
                exit(1);
            break;
        case INST_MOVEB:
            if (!handle_movX(MOVEB))
                exit(1);
            break;
        case INST_MOVEW:
            if (!handle_movX(MOVEW))
                exit(1);
            break;
        case INST_MOVED:
            if (!handle_movX(MOVED))
                exit(1);
            break;
        case INST_INTR:
            if (!handle_intr())
                exit(1);
            break;
        }
    }
}

bool masm::Parser::handle_defines(DataType t, bool _const)
{
    Token id, value;
    bool has_value = true;
    Variable var;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected an identifier after a definition key.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    id = res.value();
    if (id.type != IDENTIFIER)
    {
        log(fname, "Expected an identifier after a definition key.", l.get_line_st(), l.get_col_st());
        exit(1);
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

bool masm::Parser::add_variable(Variable v)
{
    // if the variable already exists, throw error
    auto res = symtable.vars.find(v.name);
    if (res != symtable.vars.end())
    {
        // redefinition
        Variable tmp = res->second;
        log(fname, "Redefinition of variable \"" + v.name, l.get_line_st(), l.get_col_st());
        exit(1);
    }
    symtable.vars[v.name] = v;
    return true;
}

bool masm::Parser::add_const(Variable v)
{
    // if the variable already exists, throw error
    auto res = symtable._const_list.find(v.name);
    if (res != symtable._const_list.end())
    {
        // redefinition
        Variable tmp = res->second;
        log(fname, "Redefinition of variable \"" + v.name, l.get_line_st(), l.get_col_st());
        exit(1);
    }
    symtable._const_list[v.name] = v;
    return true;
}

bool masm::Parser::handle_strings()
{
    Token id;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected an identifier after a definition key.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    id = res.value();
    if (id.type != IDENTIFIER)
    {
        log(fname, "Expected an identifier after a definition key.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    l.set_flag_ignore_dots(true);
    Token _t;
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "A string cannot have a default value..", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    _t = res.value();
    std::string final_val;
    if (_t.type == OPER_DOT || _t.type != STR)
    {
        log(fname, "The strings should start with a string", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    final_val += _t.val;
    res = l.next_token();
    while (res.has_value() && (_t = res.value()).type == OPER_DOT)
    {
        res = l.next_token();
        if (!res.has_value())
        {
            log(fname, "Expected a value or a string after the '.' operator.", l.get_line_st(), l.get_col_st());
            exit(1);
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
            log(fname, "There can only be either integers or strings after the '.' operator.", l.get_line_st(), l.get_col_st());
            exit(1);
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

bool masm::Parser::handle_names(bool _proc)
{
    Token t;
    if (_proc)
    {
        auto r = l.next_token();
        if (!r.has_value())
        {
            log(fname, "Missing an identifier after the 'proc' keyword.", l.get_line_st(), l.get_col_st());
            return false;
        }
        t = r.value();
        if (t.type != IDENTIFIER)
        {
            log(fname, "Missing an identifier after the 'proc' keyword.", l.get_line_st(), l.get_col_st());
            return false;
        }
    }
    // check if the label already exists
    auto in_pl = proc_list.find(t.val);
    auto in_label = lbl_list.find(t.val);
    bool pl = in_pl == proc_list.end();
    bool lbl = in_label == lbl_list.end();
    if (_proc)
    {
        if (!pl)
        {
            // redeclaration
            log(fname, "Redeclaration of proc \"" + t.val + "\".", l.get_line_st(), l.get_col_st());
            return false;
        }
        if (!lbl)
        {
            // The label was defined before the procedure was declared
            log(fname, "Declaration of proc \"" + t.val + "\" after the definition of its label.", l.get_line_st(), l.get_col_st());
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
                log(fname, "Redefinition of procedure label \"" + t.val + "\".", l.get_line_st(), l.get_col_st());
                return false;
            }
        }
        else
        {
            if (!lbl)
            {
                // redefinition
                log(fname, "Redefinition of procedure label \"" + t.val + "\".", l.get_line_st(), l.get_col_st());
                return false;
            }
        }
    }
    std::unique_ptr<Base> node = std::make_unique<NodeName>();
    NodeName *n = (NodeName *)node.get();
    n->oper = t.val;
    Node _n;
    _n.kind = (_proc) ? PROC_DECLR : LABEL;
    _n.node = std::move(node);
    nodes.push_back(std::move(_n));
    if (_n.kind == PROC_DECLR)
        proc_list[t.val] = false;
    else
    {
        lbl_list.insert(t.val);
        if (!pl)
            proc_list[t.val] = true;
    }
    return true;
}

void masm::Parser::handle_one(NodeKind k)
{
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.kind = k;
    nodes.push_back(std::move(node));
}

/**
 *  REG REG
 *  REG IMM
 *  REG VARIABLE
 *  REG CONSTANT
 *  REG EXPR
 */
bool masm::Parser::handle_arithmetic(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeArithmetic>();
    NodeArithmetic *a = (NodeArithmetic *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the arithmetic instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the arithmetic instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register, variable or immediate here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        node.kind = (NodeKind)(k + 2);
        a->second_oper = t.val;
        break;
    }
    case EXPR:
    {
        node.kind = (NodeKind)(k + 3);
        a->second_oper = t.expr;
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
        log(fname, "Expected a register, variable or immediate here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    }
    nodes.push_back(std::move(node));
    return true;
}

/**
 * REG REG
 */
bool masm::Parser::handle_arithmetic_float(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeArithmetic>();
    NodeArithmetic *a = (NodeArithmetic *)node.node.get();
    auto res = l.next_token();
    node.kind = (k);
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the floating-point arithmetic instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the floating-point arithmetic instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register or a variable here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        a->second_oper = regr_map.find(t.type)->second;
    else if (t.type == IDENTIFIER)
    {
        a->second_oper = t.val;
        node.kind = (NodeKind)(k + 8);
    }
    else
    {
        log(fname, "Expected a register or a variable here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true;
}

/**
 * REG REG
 * REG IMM
 * REG VAR
 * REG EXPR
 * REG CONST
 * REG LABEL
 */
bool masm::Parser::handle_mov(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeMov>();
    NodeMov *a = (NodeMov *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the mov instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the mov instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register, variable or immediate here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
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
    case EXPR:
    {
        node.kind = (NodeKind)(k + 3);
        a->second_oper = t.expr;
        break;
    }
    case NUM_FLOAT:
    {
        node.kind = k;
        a->second_oper = t.val;
        a->is_float = true;
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
        log(fname, "Expected a register, variable or immediate here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    }
    nodes.push_back(std::move(node));
    return true;
}

/**
 * REG REG
 */
bool masm::Parser::handle_movX(NodeKind k)
{
    Token t;
    Node node;
    node.kind = k;
    node.node = std::make_unique<NodeMov>();
    // we don't really need info for this node and so we ignore the columns and such metadata
    NodeMov *a = (NodeMov *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the mov instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the mov instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    // now for the second register operand
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the first register operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map.find(t.type)->second;
    t = res.value();
    if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        node.kind = (NodeKind)(k);
        a->second_oper = regr_map.find(t.type)->second;
    }
    else
    {
        log(fname, "Expected a register here after the first register operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true; // that's it!
}

/**
 * LABELS ONLY
 */
bool masm::Parser::handle_jmp(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.kind = k;
    node.node = std::make_unique<NodeName>();
    NodeName *n = (NodeName *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a label here after the branch instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (t.type != IDENTIFIER)
    {
        log(fname, "Expected a label here after the branch instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    n->oper = t.val;
    nodes.push_back(std::move(node));
    return true;
}

/**
 * LABELS
 * REGR
 */
bool masm::Parser::handle_call()
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeName>();
    NodeName *n = (NodeName *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a label here or a register after the branch instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (t.type != IDENTIFIER && !(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a label here or a register after the branch instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    node.kind = (t.type == IDENTIFIER) ? CALL : CALL_REG;
    if (t.type == IDENTIFIER)
        n->oper = t.val;
    else
        n->oper = regr_map[(t.type)];
    nodes.push_back(std::move(node));
    return true;
}

// log(fname, "Expected a register here after the arithmetic instruction.", l.get_line_st(), l.get_col_st());
