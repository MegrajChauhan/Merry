#include "worker.hpp"

void masm::Parser::setup_parser(std::string filename)
{
    fname = filename;
    filename = std::filesystem::current_path() / filename;
    if (used_files.find(filename) != used_files.end())
        return;
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
    evaluator.add_table(&symtable);
    evaluator.add_addr(&data_addr);
    used_files.insert(filename);
}

void masm::Parser::parse()
{
    if (fconts.empty())
        return;
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
        if (skip)
        {
            switch (t.type)
            {
            case KEY_DEFINED:
                handle_defined();
                break;
            case KEY_NDEFINED:
                handle_ndefined();
                break;
            case KEY_END:
            {
                if (_end_queue.empty())
                {
                    note("It may seem that a stary END keyword was found.");
                    exit(1);
                }
                _end_queue.pop_back();
                if (_end_queue.empty())
                    skip = false;
                break;
            }
            }
            continue;
        }
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
            old_tok = t;
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
            if (!handle_mov(CMP_IMM))
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
        case INST_CMPF:
            if (!handle_movX(CMPF))
                exit(1);
            break;
        case INST_CMPLF:
            if (!handle_movX(CMPLF))
                exit(1);
            break;
        case INST_GVA:
            if (!handle_load_store(GVA))
                exit(1);
            break;
        case KEY_DEPENDS:
            handle_depends();
            break;
        case KEY_DEFINED:
            handle_defined();
            break;
        case KEY_NDEFINED:
            handle_ndefined();
            break;
        case KEY_ENTRY:
            handle_entry();
            break;
        case KEY_EEPE:
            handle_eepe();
            break;
        case KEY_TEEPE:
            handle_teepe();
            break;
        case KEY_END:
        {
            if (_end_queue.empty())
            {
                note("It may seem that a stary END keyword was found.");
                exit(1);
            }
            _end_queue.pop_back();
            if (_end_queue.empty())
                skip = false;
            break;
        }
        }
    }
    if (!_end_queue.empty())
    {
        note("It may seem that one END keyword is missing.");
        exit(1);
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
    Token t = old_tok;
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
    bool pl = (in_pl != proc_list.end());
    bool lbl = (in_label != lbl_list.end());
    if (_proc)
    {
        // should be a procedure
        if (pl) // if in proc list
        {
            // redeclaration
            log(fname, "Redeclaration of proc \"" + t.val + "\".", l.get_line_st(), l.get_col_st());
            return false;
        }
        if (lbl)
        {
            // The label was defined before the procedure was declared
            log(fname, "Declaration of proc \"" + t.val + "\" after the definition of its label.", l.get_line_st(), l.get_col_st());
            return false;
        }
    }
    else
    {           // should not be a procedure
        if (pl) // if in proc list
        {
            // could be a definition
            if (lbl) // already defined
            {
                // redefinition
                log(fname, "Redefinition of procedure label \"" + t.val + "\".", l.get_line_st(), l.get_col_st());
                return false;
            }
        }
        else
        { // not in proc list
            // could be a label declaration
            if (lbl) // already defined
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
        if (pl)
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
    a->reg = regr_map[t.type];
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
    if (k >= IADD_IMM && k <= IMOD_EXPR)
        a->is_signed = true;
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
    a->reg = regr_map[t.type];
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register or a constant here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        a->second_oper = regr_map[t.type];
    else if (t.type == IDENTIFIER)
    {
        a->second_oper = t.val;
        node.kind = (NodeKind)(k + 8);
    }
    else
    {
        log(fname, "Expected a register or a constant here after the first operand.", l.get_line_st(), l.get_col_st());
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
        log(fname, "Expected a register here after opcode.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the opcode.", l.get_line_st(), l.get_col_st());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register, variable or immediate here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map[t.type];
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
        log(fname, "Expected a register here after the movement instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the movement instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    // now for the second register operand
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the first register operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map[t.type];
    t = res.value();
    if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        a->second_oper = regr_map[t.type];
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

/**
 * REG IMM
 * REG CONST
 * REG EXPR
 * REG VAR
 * REG REG
 */
bool masm::Parser::handle_sva_svc(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeStack>();
    NodeStack *a = (NodeStack *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the stack instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the stack instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register, variable or immediate here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->reg = regr_map[t.type];
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
 * IMM
 * VARS
 * CONST
 * REGR
 * EXPR
 * LABELS
 */
bool masm::Parser::handle_push_pop(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeStack>();
    auto n = (NodeStack *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a label, or a register, or an identifier or an immediate after the STACK instruction", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        node.kind = (NodeKind)(k + 2);
        n->second_oper = t.val;
        break;
    }
    case EXPR:
    {
        if (k == POP_IMM)
        {
            log(fname, "POP instruction doesn't accept expressions.", l.get_line_st(), l.get_col_st());
            return false;
        }
        node.kind = (NodeKind)(k + 3);
        n->second_oper = t.expr;
        break;
    }
    case NUM_FLOAT:
    case NUM_INT:
    {
        if (k == POP_IMM)
        {
            log(fname, "POP instruction doesn't accept immediates.", l.get_line_st(), l.get_col_st());
            return false;
        }
        node.kind = (k);
        n->second_oper = t.val;
        break;
    }
    default:
        if (t.type >= KEY_Ma && t.type <= KEY_Mm5)
        {
            node.kind = (NodeKind)(k + 1);
            n->second_oper = regr_map[t.type];
            break;
        }
        log(fname, "Expected a label, identifier,or an immediate here that after the STACK instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_single_regr(NodeKind k)
{
    Token t;
    Node node;
    node.node = std::make_unique<NodeName>();
    auto n = (NodeName *)node.node.get();
    node.kind = k;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here as the operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here as the operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    n->oper = regr_map[t.type];
    nodes.push_back(std::move(node));
    return true;
}

/**
 * REG REG
 * REG IMM
 * REG CONST
 * REG EXPR
 */
bool masm::Parser::handle_logical_inst(NodeKind k, bool limit)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeLogical>();
    auto n = (NodeLogical *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register or an immediate after the LOGICAL instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    n->reg = regr_map[t.type];
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected an immediate or another register after the LOGICAL instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case EXPR:
    {
        node.kind = (NodeKind)(k + 3);
        n->second_oper = t.expr;
        break;
    }
    case IDENTIFIER:
    {
        node.kind = (NodeKind)(k + 2);
        n->second_oper = t.val;
        break;
    }
    case NUM_INT:
    {
        node.kind = (k);
        n->second_oper = t.val;
        break;
    }
    default:
        if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            node.kind = (NodeKind)(k + 1);
            n->second_oper = regr_map[t.type];
            break;
        }
        log(fname, "Expected an immediate or a register here that after the LOGICAL instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_lea()
{
    Token t;
    Register r[4];
    std::optional<Token> res;
    for (size_t i = 0; i < 4; i++)
    {
        res = l.next_token();
        if (!res.has_value())
        {
            log(fname, "Expected a register here in the LEA instruction.", l.get_line_st(), l.get_col_st());
            return false;
        }
        t = res.value();
        if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            log(fname, "Expected a register here in the LEA instruction.", l.get_line_st(), l.get_col_st());
            return false;
        }
        r[i] = regr_map[t.type];
    }
    Node node;
    node.kind = LEA;
    node.node = std::make_unique<NodeLea>();
    auto n = (NodeLea *)node.node.get();
    n->dest = r[0];
    n->base = r[1];
    n->ind = r[2];
    n->scale = r[3];
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_load_store(NodeKind k, bool atm)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeLoadStore>();
    auto n = (NodeLoadStore *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here in the MEM instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here in the MEM instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    n->reg = regr_map[t.type];
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register or variable here in the MEM instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
    {
        node.kind = (NodeKind)(k + 1);
        n->second_oper = t.val;
        break;
    }
    default:
        if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            log(fname, "Expected an identifier or a register here after the MEM instruction.", l.get_line_st(), l.get_col_st());
            return false;
        }
        node.kind = k;
        n->second_oper = regr_map[t.type];
        break;
    }
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_atm()
{
    Token t;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected an instruction after the ATM keyword.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case INST_LOADB:
        if (!handle_load_store(ALOADB_REG, true))
            return false;
        break;
    case INST_LOADW:
        if (!handle_load_store(ALOADW_REG, true))
            return false;
        break;
    case INST_LOADD:
        if (!handle_load_store(ALOADD_REG, true))
            return false;
        break;
    case INST_LOADQ:
        if (!handle_load_store(ALOADQ_REG, true))
            return false;
        break;
    case INST_STOREB:
        if (!handle_load_store(ASTOREB_REG, true))
            return false;
        break;
    case INST_STOREW:
        if (!handle_load_store(ASTOREW_REG, true))
            return false;
        break;
    case INST_STORED:
        if (!handle_load_store(ASTORED_REG, true))
            return false;
        break;
    case INST_STOREQ:
        if (!handle_load_store(ASTOREQ_REG, true))
            return false;
        break;
    default:
        log(fname, "Unknown atomic instruction. Only LOADS and STORES are atomic.", l.get_line_st(), l.get_col_st());
        return false;
    }
    return true;
}

bool masm::Parser::handle_cmpxchg()
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeCmpxchg>();
    auto n = (NodeCmpxchg *)node.node.get();
    Register r[2];
    std::optional<Token> res;
    for (size_t i = 0; i < 2; i++)
    {
        res = l.next_token();
        if (!res.has_value())
        {
            log(fname, "Expected a register here in the CMPXCHG instruction.", l.get_line_st(), l.get_col_st());
            return false;
        }
        t = res.value();
        if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
        {
            log(fname, "Expected a register here in the CMPXCHG instruction.", l.get_line_st(), l.get_col_st());
            return false;
        }
        r[i] = regr_map[t.type];
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register or a variable here in the CMPXCHG instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
        node.kind = CMPXCHG;
        n->var = t.val;
        break;
    default:
        if (t.type >= KEY_Ma && t.type <= KEY_Mm5)
        {
            node.kind = CMPXCHG_REGR;
            n->var = regr_map[t.type];
            break;
        }
        log(fname, "Expected a register or a variable here in the CMPXCHG instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    n->reg1 = r[0];
    n->reg2 = r[1];
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_sio(NodeKind k)
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeName>();
    auto n = (NodeName *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected an identifier after the SIO instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case IDENTIFIER:
        node.kind = (k);
        n->oper = t.val;
        break;
    default:
        if (t.type >= KEY_Ma && t.type <= KEY_Mm5)
        {
            node.kind = (NodeKind)(k + 1);
            n->oper = regr_map[t.type];
            break;
        }
        log(fname, "Expected an identifier after the SIO instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_excg(NodeKind k)
{
    Token t;
    Node node;
    node.kind = k;
    node.node = std::make_unique<NodeExcg>();
    NodeExcg *a = (NodeExcg *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the EXCG instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    if (!(t.type >= KEY_Ma && t.type <= KEY_Mm5))
    {
        log(fname, "Expected a register here after the EXCG instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a register here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    a->r1 = regr_map[t.type];
    t = res.value();
    if ((t.type >= KEY_Ma && t.type <= KEY_Mm5))
        a->r2 = regr_map[t.type];
    else
    {
        log(fname, "Expected a register here after the first operand.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true;
}

bool masm::Parser::handle_intr()
{
    Token t;
    Node node;
    node.line = l.get_line();
    node._file = file;
    node.node = std::make_unique<NodeIntr>();
    auto n = (NodeIntr *)node.node.get();
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a constant, or an immediate after the INTR instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    t = res.value();
    switch (t.type)
    {
    case EXPR:
    {
        node.kind = INTR_EXPR;
        n->val = t.expr;
        break;
    }
    case IDENTIFIER:
    {
        node.kind = INTR_VAR;
        n->val = t.val;
        break;
    }
    case NUM_INT:
    {
        node.kind = (INTR);
        n->val = t.val;
        break;
    }
    default:
        log(fname, "Expected a constant, or an immediate after the INTR instruction.", l.get_line_st(), l.get_col_st());
        return false;
    }
    nodes.push_back(std::move(node));
    return true;
}

void masm::Parser::handle_depends()
{
    std::string _file = l.get_a_group();
    if (_file.empty() && l.eof())
    {
        log(fname, "Expected a BUILD file after 'depends' that needs to be read.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    Parser child;
    if (_std_paths.find(_file) == _std_paths.end())
        _file = std::filesystem::current_path() / _file;
    else
        _file = _std_paths.find(_file)->second;
    child.setup_parser(_file);
    child.parse();
}

void masm::Parser::handle_defined()
{
    auto tok = l.next_token();
    _end_queue.push_back(true);
    if (skip)
        return;
    if (!tok.has_value())
    {
        note("Expected a constant after 'defined'.");
        exit(1);
    }
    Token t = tok.value();
    if (t.type != IDENTIFIER && t.type != EXPR)
    {
        note("Expected a constant or expression after 'defined'.");
        exit(1);
    }
    if (t.type == EXPR)
    {
        evaluator.add_expr(t.expr);
        auto _r = evaluator.evaluate(true);
        // In the case of defined an ndefined, the expression must evaluate to either 1 or 0
        // In defined, 1 would mean that the expression is evaluated but otherwise in the case of ndefined
        if (!_r.has_value())
        {
            note("In file: " + fname);
            exit(1);
        }
        if (std::stoull(_r.value()) == 1)
            return;
    }
    else
    {
        if (symtable._const_list.find(t.val) == symtable._const_list.end())
        {
            skip = true;
        }
        // a constant hence it is defined
        return;
    }
}

void masm::Parser::handle_ndefined()
{
    auto tok = l.next_token();
    _end_queue.push_back(true);
    if (skip)
        return;
    if (!tok.has_value())
    {
        note("Expected a constant after 'defined'.");
        exit(1);
    }
    Token t = tok.value();
    if (t.type != IDENTIFIER)
    {
        note("Expected a constant after 'defined'.");
        exit(1);
    }
    if (t.type == EXPR)
    {
        evaluator.add_expr(t.expr);
        auto _r = evaluator.evaluate(true);
        if (!_r.has_value())
        {
            note("While evaluating expression here in file " + fname);
            exit(1);
        }
        if (std::stoull(_r.value()) == 0)
            return;
    }
    else if ((symtable._const_list.find(t.val) == symtable._const_list.end()))
    {
        return;
    }
    skip = true;
}

void masm::Parser::handle_entry()
{
    Token t;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a label or a procedure name after 'entry'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    t = res.value();
    if (t.type != IDENTIFIER)
    {
        log(fname, "Expected a label or a procedure name after 'entry'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    entries.push_back(t.val);
}

void masm::Parser::handle_eepe()
{
    Token t;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected an integer value after 'eepe'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    t = res.value();
    if (t.type != NUM_INT)
    {
        log(fname, "Expected an integer value after 'eepe'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    eepe = t.val;
}

void masm::Parser::handle_teepe()
{
    Token t;
    auto res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a label after 'teepe'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    t = res.value();
    if (t.type != IDENTIFIER)
    {
        log(fname, "Expected a label after 'teepe'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    std::string id = t.val;
    res = l.next_token();
    if (!res.has_value())
    {
        log(fname, "Expected a value after label after 'teepe'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    t = res.value();
    if (t.type != NUM_INT)
    {
        log(fname, "Expected a value after label after 'teepe'.", l.get_line_st(), l.get_col_st());
        exit(1);
    }
    teepe[id] = t.val;
}

void masm::Parser::confirm_entries()
{
    if (entries.empty())
    {
        note("No entries were provided. At least one entry must be provided.");
        exit(1);
    }
    for (auto e : entries)
    {
        if (lbl_list.find(e) == lbl_list.end())
        {
            note("The entry '" + e + "' doesn't exist.");
            exit(1);
        }
    }
    for (auto te : teepe)
    {
        if (lbl_list.find(te.first) == lbl_list.end())
        {
            note("The entry '" + te.first + "' doesn't exist.");
            exit(1);
        }
    }
}

void masm::Parser::analyse_proc()
{
    for (auto p : proc_list)
    {
        if (!p.second)
        {
            note("The procedure \"" + p.first + "\" was not defined.");
            exit(1);
        }
    }
}

void masm::Parser::parser_confirm_info()
{
    analyse_proc();
    confirm_entries();
    if (eepe == "0")
        eepe = "1";
    if (!evaluate_data(&symtable, &data_addr, &data, &str))
        exit(1);
    analyse_nodes();
    make_label_address();
}

void masm::Parser::add_for_codegen(CodeGen *g)
{
    g->data_addr = &data_addr;
    g->lbl_addr = &label_addr;
    g->data = &data;
    g->str = &str;
    g->nodes = &nodes;
    g->table = &symtable;
}

void masm::Parser::add_for_emit(Emit *e)
{
    e->eepe = &eepe;
    e->entries = &entries;
    e->teepe = &teepe;
}

void masm::Parser::make_label_address()
{
    size_t i = 0;
    for (auto &l : nodes)
    {
        switch (l.kind)
        {
        case LABEL:
        {
            NodeName *n = (NodeName *)l.node.get();
            (label_addr)[std::get<std::string>(n->oper)] = i;
            break;
        }
        default:
            if (l.kind >= NOP)
                i++;
            switch (l.kind)
            {
            case MOVL_IMM:
            case PUSH_IMM:
            case POP_IMM: // again, but for the joke this time
            case AND_IMM:
            case OR_IMM:
            case XOR_IMM:
            case CMP_IMM:
                i++;
                break;
            }
        }
    }
}

void masm::Parser::analyse_nodes()
{
    for (auto &n : nodes)
    {
        switch (n.kind)
        {
        case ADD_MEM:
        case SUB_MEM:
        case MUL_MEM:
        case DIV_MEM:
        case MOD_MEM:
        case FADD_MEM:
        case FSUB_MEM:
        case FMUL_MEM:
        case FDIV_MEM:
        case LFADD_MEM:
        case LFSUB_MEM:
        case LFMUL_MEM:
        case LFDIV_MEM:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            if (symtable.vars.find(var_name) == symtable.vars.end())
            {
                // could be a constant
                if (symtable._const_list.find(var_name) == symtable._const_list.end())
                {
                    note("The variable '" + var_name + "' doesn't exist.");
                    exit(1);
                }
                else
                {
                    if (n.kind >= FADD_MEM && n.kind <= LFDIV_MEM)
                    {
                        note("FLOATING POINT Arithmetic instructions do not take constants as operands.");
                        exit(1);
                    }
                    else
                        n.kind = (NodeKind)(n.kind - 2);
                    _n->second_oper = symtable._const_list.find(var_name)->second.value;
                }
            }
            break;
        }
        case GVA_VAR:
        case LOADB_VAR:
        case LOADW_VAR:
        case LOADD_VAR:
        case LOADQ_VAR:
        case STOREB_VAR:
        case STOREW_VAR:
        case STORED_VAR:
        case STOREQ_VAR:
        case ALOADB_VAR:
        case ALOADW_VAR:
        case ALOADD_VAR:
        case ALOADQ_VAR:
        case ASTOREB_VAR:
        case ASTOREW_VAR:
        case ASTORED_VAR:
        case ASTOREQ_VAR:
        {
            auto _n = (NodeLoadStore *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            if (symtable.vars.find(var_name) == symtable.vars.end())
            {
                note("The variable '" + var_name + "' doesn't exist.");
                exit(1);
            }
            break;
        }
        case ADD_EXPR:
        case SUB_EXPR:
        case MUL_EXPR:
        case DIV_EXPR:
        case MOD_EXPR:
        case IADD_EXPR:
        case ISUB_EXPR:
        case IMUL_EXPR:
        case IDIV_EXPR:
        case IMOD_EXPR:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = v.value();
            break;
        }
        case AND_EXPR:
        case OR_EXPR:
        case XOR_EXPR:
        case LSHIFT_EXPR:
        case RSHIFT_EXPR:
        case CMP_EXPR:
        {
            auto _n = (NodeLogical *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = v.value();
            _n->is_float = true;
            break;
        }
        case MOV_EXPR:
        case MOVL_EXPR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = MOVL_IMM;
            _n->second_oper = v.value();
            if (!evaluator.was_addr())
                _n->is_float = true;
            else
                _n->is_float = false;
            break;
        }
        case MOVSXB_EXPR:
        case MOVSXW_EXPR:
        case MOVSXD_EXPR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = v.value();
            break;
        }
        case SVA_EXPR:
        case SVC_EXPR:
        {
            auto _n = (NodeStack *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = v.value();
            break;
        }
        case PUSH_EXPR:
        {
            auto _n = (NodeStack *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->second_oper);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 3);
            _n->second_oper = v.value();
            break;
        }
        case PUSH_VAR:
        case POP_VAR:
        {
            auto _n = (NodeStack *)n.node.get();
            auto var_name = std::get<std::string>(_n->second_oper);
            if (symtable.vars.find(var_name) == symtable.vars.end())
            {
                if (n.kind == PUSH_VAR)
                {
                    if (symtable._const_list.find(var_name) != symtable._const_list.end())
                    {
                        n.kind = PUSH_IMM;
                        _n->second_oper = symtable._const_list.find(var_name)->second.value;
                        break;
                    }
                }
                if ((n.kind == PUSH_VAR) && (lbl_list.find(var_name) != lbl_list.end()))
                {
                    n.kind = PUSH_IMM;
                    _n->_is_lbl = true;
                    break;
                }
                note("The variable '" + var_name + "' doesn't exist.");
                exit(1);
            }
            break;
        }
        case IADD_VAR:
        case ISUB_VAR:
        case IMUL_VAR:
        case IDIV_VAR:
        case IMOD_VAR:
        {
            auto _n = (NodeArithmetic *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            auto r = symtable._const_list.find(var_name);
            // must be a constant
            if (r == symtable._const_list.end())
            {
                note("The constant '" + var_name + "' doesn't exist.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 2);
            _n->second_oper = r->second.value;
            break;
        }
        case AND_VAR:
        case OR_VAR:
        case XOR_VAR:
        case LSHIFT_VAR:
        case RSHIFT_VAR:
        {
            auto _n = (NodeLogical *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            auto r = symtable._const_list.find(var_name);
            // must be a constant
            if (r == symtable._const_list.end())
            {
                note("The constant '" + var_name + "' doesn't exist.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 2);
            _n->second_oper = r->second.value;
            _n->is_float = r->second.type == FLOAT;
            break;
        }
        case CMP_VAR:
        {
            auto _n = (NodeLogical *)n.node.get();
            std::string var_name = std::get<std::string>(_n->second_oper);
            auto r = symtable.vars.find(var_name);
            if (r == symtable.vars.end())
            {
                if (symtable._const_list.find(var_name) == symtable._const_list.end())
                {
                    note("The variable '" + var_name + "' doesn't exist.");
                    exit(1);
                }
                else
                {
                    n.kind = (NodeKind)(n.kind - 2);
                    _n->second_oper = symtable._const_list.find(var_name)->second.value;
                }
            }
            break;
        }
        case MOV_VAR:
        case MOVL_VAR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto var_name = std::get<std::string>(_n->second_oper);
            auto r = symtable._const_list.find(var_name);
            if (r == symtable._const_list.end())
            {
                if ((label_addr.find(var_name) != label_addr.end()))
                {
                    n.kind = MOVL_IMM;
                    _n->is_lbl = true;
                    break;
                }
                auto _r = symtable.vars.find(var_name);
                if (_r == symtable.vars.end())
                {
                    note("The variable '" + var_name + "' doesn't exist.");
                    exit(1);
                }
                n.kind = MOV_VAR;
                break;
            }
            n.kind = MOVL_IMM;
            _n->second_oper = r->second.value;
            _n->is_float = r->second.type == FLOAT;
            break;
        }
        case SVA_VAR:
        case SVC_VAR:
        {
            auto _n = (NodeStack *)n.node.get();
            auto var_name = std::get<std::string>(_n->second_oper);
            auto r = symtable._const_list.find(var_name);
            if (r == symtable._const_list.end())
            {
                auto _r = symtable.vars.find(var_name);
                if (_r == symtable.vars.end())
                {
                    note("The variable '" + var_name + "' doesn't exist.");
                    exit(1);
                }
            }
            else
            {
                n.kind = (NodeKind)(n.kind - 2);
                _n->second_oper = r->second.value;
            }
            break;
        }
        case MOVSXB_VAR:
        case MOVSXW_VAR:
        case MOVSXD_VAR:
        {
            auto _n = (NodeMov *)n.node.get();
            auto var_name = std::get<std::string>(_n->second_oper);
            auto r = symtable._const_list.find(var_name);
            if (r == symtable._const_list.end())
            {
                note("The constant '" + var_name + "' doesn't exist.");
                exit(1);
            }
            n.kind = (NodeKind)(n.kind - 2);
            _n->second_oper = r->second.value;
            break;
        }
        case JMP:
        case JNZ:
        case JZ:
        case JNE:
        case JE:
        case JNC:
        case JC:
        case JNO:
        case JO:
        case JNN:
        case JN:
        case JNG:
        case JG:
        case JNS:
        case JS:
        case JGE:
        case JSE:
        case SETE:
        case LOOP:
        case CALL:
        {
            auto _n = (NodeName *)n.node.get();
            std::string name = std::get<std::string>(_n->oper);
            if (lbl_list.find(name) == lbl_list.end())
            {
                note("This label to branch into doesn't exist: " + name);
                exit(1);
            }
            break;
        }
        case CMPXCHG:
        {
            auto _n = (NodeCmpxchg *)n.node.get();
            if (symtable.vars.find(std::get<std::string>(_n->var)) == symtable.vars.end())
            {
                note("This variable doesn't exist: " + std::get<std::string>(_n->var));
                exit(1);
            }
            break;
        }
        case SIN:
        case SOUT:
        {
            auto _n = (NodeIO *)n.node.get();
            if (symtable.vars.find(std::get<std::string>(_n->oper)) == symtable.vars.end())
            {
                note("This variable doesn't exist: " + std::get<std::string>(_n->oper));
                exit(1);
            }
            break;
        }
        case INTR_VAR:
        {
            auto _n = (NodeIntr *)n.node.get();
            auto name = std::get<std::string>(_n->val);
            if (symtable._const_list.find(name) == symtable._const_list.end())
            {
                note("This constant doesn't exist: " + name);
                exit(1);
            }
            n.kind = INTR;
            _n->val = symtable._const_list.find(name)->second.value;
            break;
        }
        case INTR_EXPR:
        {
            auto _n = (NodeIntr *)n.node.get();
            auto expr = std::get<std::vector<Token>>(_n->val);
            evaluator.add_expr(expr);
            auto v = evaluator.evaluate();
            if (!v.has_value())
            {
                note("While evaluating the expression here.");
                exit(1);
            }
            n.kind = INTR;
            _n->val = v.value();
            break;
        }
        }
    }
}