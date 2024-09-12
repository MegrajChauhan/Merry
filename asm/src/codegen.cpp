#include "codegen.hpp"

bool masm::evaluate_consts(SymbolTable *table, Expr *e)
{
    for (auto c : table->_const_list)
    {
        Variable var = c.second;
        if (var.is_expr)
        {
            e->add_expr(var.expr);
            if (!e->evaluate(true))
            {
                note("While evaluating the constant " + var.name);
                return false;
            }
        }
    }
    return true;
}

bool masm::evaluate_data(SymbolTable *table, std::unordered_map<std::string, size_t> *daddr, std::vector<uint8_t> *data, std::vector<uint8_t> *str_data)
{
    size_t start = 0;
    Expr e;
    e.add_addr(daddr);
    e.add_table(table);
    if (!evaluate_consts(table, &e))
        return false;
    for (auto var : table->vars)
    {
        Variable v = var.second;
        if (v.is_expr)
        {
            e.add_expr(v.expr);
            auto res = e.evaluate();
            if (!res.has_value())
            {
                note("While evaluating the variable " + v.name);
                return false;
            }
            v.value = res.value();
        }
        switch (v.type)
        {
        case BYTE:
        {
            (*daddr)[v.name] = start;
            data->push_back(std::stoull(v.value) & 0xFF);
            start++;
            break;
        }
        case WORD:
        {
            (*daddr)[v.name] = start;
            uint16_t val = std::stoull(v.value);
            data->push_back(val & 0xFF);
            data->push_back((val >> 8) & 0xFF);
            start += 2;
            break;
        }
        case DWORD:
        {
            (*daddr)[v.name] = start;
            uint32_t val = std::stoull(v.value);
            data->push_back(val & 0xFF);
            data->push_back((val >> 8) & 0xFF);
            data->push_back((val >> 16) & 0xFF);
            data->push_back((val >> 24) & 0xFF);
            start += 4;
            break;
        }
        case QWORD:
        {
            (*daddr)[v.name] = start;
            uint64_t val = std::stoull(v.value);
            data->push_back(val & 0xFF);
            data->push_back((val >> 8) & 0xFF);
            data->push_back((val >> 16) & 0xFF);
            data->push_back((val >> 24) & 0xFF);
            data->push_back((val >> 32) & 0xFF);
            data->push_back((val >> 40) & 0xFF);
            data->push_back((val >> 48) & 0xFF);
            data->push_back((val >> 56) & 0xFF);
            start += 8;
            break;
        }
        case FLOAT:
        {
            (*daddr)[v.name] = start;
            uint32_t val = (uint32_t)(std::stof(v.value));
            data->push_back(val & 0xFF);
            data->push_back((val >> 8) & 0xFF);
            data->push_back((val >> 16) & 0xFF);
            data->push_back((val >> 24) & 0xFF);
            start += 4;
            break;
        }
        case LFLOAT:
        {
            (*daddr)[v.name] = start;
            uint64_t val = (uint64_t)(std::stod(v.value));
            data->push_back(val & 0xFF);
            data->push_back((val >> 8) & 0xFF);
            data->push_back((val >> 16) & 0xFF);
            data->push_back((val >> 24) & 0xFF);
            data->push_back((val >> 32) & 0xFF);
            data->push_back((val >> 40) & 0xFF);
            data->push_back((val >> 48) & 0xFF);
            data->push_back((val >> 56) & 0xFF);
            start += 8;
            break;
        }
        case RESB:
        {
            (*daddr)[v.name] = start;
            size_t len = std::stoull(v.value);
            for (size_t i = 0; i < len; i++)
                data->push_back(0);
            start += len;
            break;
        }
        case RESW:
        {
            (*daddr)[v.name] = start;
            size_t len = std::stoull(v.value) * 2;
            for (size_t i = 0; i < len; i++)
                data->push_back(0);
            start += len;
            break;
        }
        case RESD:
        {
            (*daddr)[v.name] = start;
            size_t len = std::stoull(v.value) * 4;
            for (size_t i = 0; i < len; i++)
                data->push_back(0);
            start += len;
            break;
        }
        case RESQ:
        {
            (*daddr)[v.name] = start;
            size_t len = std::stoull(v.value) * 8;
            for (size_t i = 0; i < len; i++)
                data->push_back(0);
            start += len;
            break;
        }
        }
    }
    while ((data->size() % 8) != 0)
    {
        data->push_back(0);
        start++;
    }
    for (auto v : table->vars)
    {
        auto var = v.second;
        switch (var.type)
        {
        case STRING:
        {
            (*daddr)[var.name] = start;
            for (auto _c : var.value)
            {
                str_data->push_back(_c);
            }
            start += var.value.length();
            break;
        }
        }
    }
    while ((str_data->size() % 8) != 0)
        str_data->push_back(0);
}

void masm::CodeGen::generate_singles(size_t opcode)
{
    GenBinary b;
    b.bytes.b1 = opcode;
    code.push_back(b);
}

void masm::CodeGen::arithmetic_inst_imm(NodeArithmetic *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b2 = n->reg;
    b.full |= (std::stoull(std::get<std::string>(n->second_oper)) & (n->is_signed ? 0xFFFFFFFF : 0xFFFFFFFFFFFF));
    code.push_back(b);
}

void masm::CodeGen::arithmetic_inst_reg(NodeArithmetic *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(n->second_oper);
    code.push_back(b);
}

void masm::CodeGen::arithmetic_inst_mem(NodeArithmetic *n, size_t op)
{
    // This function will only be called for unsigned ones
    GenBinary b;
    b.bytes.b2 = n->reg;
    std::string v_name = std::get<std::string>(n->second_oper);
    auto res = table->vars.find(v_name);
    Variable v = res->second;
    switch (v.type)
    {
    case BYTE:
    case STRING:
    case RESB:
        b.bytes.b1 = op;
        break;
    case WORD:
    case RESW:
        b.bytes.b1 = op + 1;
        break;
    case DWORD:
    case FLOAT:
    case RESD:
        b.bytes.b1 = op + 2;
        break;
    case QWORD:
    case LFLOAT:
    case RESQ:
        b.bytes.b1 = op + 3;
        break;
    }
    size_t addr = (*data_addr)[v_name];
    b.full |= (addr & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::float_mem(NodeArithmetic *n, size_t op)
{
    GenBinary b;
    b.bytes.b2 = n->reg;
    std::string v_name = std::get<std::string>(n->second_oper);
    auto res = table->vars.find(v_name);
    size_t addr = (*data_addr)[v_name];
    b.bytes.b1 = op;
    b.full |= (addr & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::mov_imm(NodeMov *n, bool _is64)
{
    if (n->is_lbl)
    {
        mov_var(n);
        return;
    }
    GenBinary b;
    b.bytes.b1 = (_is64) ? OP_MOVE_IMM : OP_MOVE_IMM_64;
    std::string imm = std::get<std::string>(n->second_oper);
    size_t _imm = n->is_float ? (_is64 ? (size_t)std::stod(imm) : (size_t)std::stof(imm)) : std::stoull(imm);
    if (_is64)
    {
        b.bytes.b8 = n->reg;
        code.push_back(b);
        b.full = _imm;
    }
    else
    {
        b.bytes.b2 = n->reg;
        b.full |= (_imm & 0xFFFFFFFFFFFF);
    }
    code.push_back(b);
}

void masm::CodeGen::mov_reg(NodeMov *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(n->second_oper);
    code.push_back(b);
}

void masm::CodeGen::mov_var(NodeMov *n)
{
    GenBinary b;
    auto _s = std::get<std::string>(n->second_oper);
    auto _res = table->vars.find(_s);
    bool is_lbl = false;
    if (_res == table->vars.end())
    {
        if (table->_const_list.find(_s) == table->_const_list.end())
            is_lbl = true;
        else
        {
            // is a constant
            _s = table->_const_list[_s].value;
            n->second_oper = _s;
            n->is_float = table->_const_list[_s].type == FLOAT;
            mov_imm(n, true);
            return;
        }
    }
    size_t addr = is_lbl ? (*lbl_addr)[_s] : data_addr->find(_s)->second;
    b.bytes.b2 = n->reg;
    b.full |= (addr & 0xFFFFFFFFFFFF);
    if (!is_lbl)
    {
        Variable dets = _res->second;
        switch (dets.type)
        {
        case BYTE:
        case STRING:
        case RESB:
            b.bytes.b1 = OP_LOADB;
            break;
        case WORD:
        case RESW:
            b.bytes.b1 = OP_LOADW;
            break;
        case DWORD:
        case FLOAT:
        case RESD:
            b.bytes.b1 = OP_LOADD;
            break;
        case QWORD:
        case LFLOAT:
        case RESQ:
            b.bytes.b1 = OP_LOAD;
            break;
        }
    }
    else
    {
        b.full = 0;
        b.bytes.b1 = OP_MOVE_IMM_64;
        b.bytes.b8 = n->reg;
        code.push_back(b);
        b.full = (addr & 0xFFFFFFFFFFFF);
    }
    code.push_back(b);
}

void masm::CodeGen::movsx_imm(NodeMov *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    std::string imm = std::get<std::string>(n->second_oper);
    size_t _imm = std::stoull(imm);
    b.bytes.b2 = n->reg;
    b.full |= (_imm & (op == OP_MOVESX_IMM8 ? 0xFF : (op == OP_MOVESX_IMM16 ? 0xFFFF : 0xFFFFFFFF)));
    code.push_back(b);
}

void masm::CodeGen::branch(NodeName *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    if (op == OP_CALL_REG)
    {
        b.bytes.b8 = std::get<Register>(n->oper);
    }
    else
    {
        size_t addr = (*lbl_addr)[std::get<std::string>(n->oper)];
        b.full |= (addr & 0xFFFFFFFFFFFF);
    }
    code.push_back(b);
}

void masm::CodeGen::sva_svc(NodeStack *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b2 = n->reg;
    switch (op)
    {
    case OP_SVA:
    case OP_SVC:
        b.full |= (std::stoull(std::get<std::string>(n->second_oper)) & 0xFFFF);
        break;
    case OP_SVA_REG:
    case OP_SVC_REG:
        b.bytes.b8 = std::get<Register>(n->second_oper);
        break;
    case OP_SVA_MEM:
    case OP_SVC_MEM:
        b.full |= ((*data_addr)[std::get<std::string>(n->second_oper)] & 0xFFFFFFFFFFFF);
        break;
    }
    code.push_back(b);
}

void masm::CodeGen::stack(NodeStack *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    switch (op)
    {
    case OP_PUSH_IMM:
        code.push_back(b);
        if (n->_is_lbl)
            b.full = (*lbl_addr)[std::get<std::string>(n->second_oper)];
        else
            b.full = std::stoull(std::get<std::string>(n->second_oper));
        break;
    case OP_PUSH_REG:
    case OP_POP:
        b.bytes.b8 = std::get<Register>(n->second_oper);
        break;
    default:
        std::string name = std::get<std::string>(n->second_oper);
        auto res = data_addr->find(name);
        size_t addr = 0;
        if (res == data_addr->end())
            addr = (*lbl_addr)[name];
        else
        {
            addr = res->second;
            switch (table->vars.find(name)->second.type)
            {
            case BYTE:
            case STRING:
            case RESB:
                b.bytes.b1 = op;
                break;
            case WORD:
            case RESW:
                b.bytes.b1 = op + 1;
                break;
            case DWORD:
            case RESD:
            case FLOAT:
                b.bytes.b1 = op + 2;
                break;
            case QWORD:
            case RESQ:
            case LFLOAT:
                b.bytes.b1 = op + 3;
                break;
            }
        }
        b.full |= (addr & 0xFFFFFFFFFFFF);
    }
    code.push_back(b);
}

void masm::CodeGen::logical_singles(NodeName *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = std::get<Register>(n->oper);
    code.push_back(b);
}

void masm::CodeGen::logical_inst_imm(NodeLogical *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    if (op == OP_LSHIFT || op == OP_RSHIFT)
    {
        b.bytes.b7 = n->reg;
        b.full |= ((std::stoull(std::get<std::string>(n->second_oper))) & 0x40);
    }
    else
    {
        b.bytes.b8 = n->reg;
        code.push_back(b);
        b.full = (n->is_float) ? (size_t)(std::stod(std::get<std::string>(n->second_oper))) : (std::stoull(std::get<std::string>(n->second_oper)));
    }
    code.push_back(b);
}

void masm::CodeGen::logical_inst_reg(NodeLogical *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(n->second_oper);
    code.push_back(b);
}

void masm::CodeGen::cmp_var(NodeLogical *n)
{
    GenBinary b;
    std::string name = std::get<std::string>(n->second_oper);
    auto res = data_addr->find(name);
    size_t addr = res->second;
    b.full |= (addr & 0xFFFFFFFFFFFF);
    b.bytes.b2 = n->reg;
    switch (table->vars.find(name)->second.type)
    {
    case BYTE:
    case RESB:
    case STRING:
        b.bytes.b1 = OP_CMP_IMM_MEMB;
        break;
    case WORD:
    case RESW:
        b.bytes.b1 = OP_CMP_IMM_MEMW;
        break;
    case DWORD:
    case FLOAT:
    case RESD:
        b.bytes.b1 = OP_CMP_IMM_MEMD;
        break;
    case QWORD:
    case LFLOAT:
    case RESQ:
        b.bytes.b1 = OP_CMP_IMM_MEMQ;
        break;
    }
    code.push_back(b);
}

void masm::CodeGen::lea(NodeLea *n)
{
    GenBinary b;
    b.bytes.b1 = OP_LEA;
    b.bytes.b8 = n->scale;
    b.bytes.b7 = n->ind;
    b.bytes.b6 = n->base;
    b.bytes.b5 = n->dest;
    code.push_back(b);
}

void masm::CodeGen::load_store_var(NodeLoadStore *n, size_t op)
{
    GenBinary b;
    auto _s = std::get<std::string>(n->second_oper);
    size_t addr = data_addr->find(_s)->second;
    b.bytes.b1 = op;
    b.bytes.b2 = n->reg;
    b.full |= (addr & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::cmpxchg(NodeCmpxchg *n, bool _reg)
{
    GenBinary b;
    b.bytes.b1 = _reg ? OP_CMPXCHG_REGR : OP_CMPXCHG;
    b.bytes.b2 = n->reg1;
    b.bytes.b2 <<= 4;
    b.bytes.b2 |= n->reg2;
    if (_reg)
        b.bytes.b8 = std::get<Register>(n->var);
    else
    {
        size_t addr = (*data_addr)[std::get<std::string>(n->var)];
        b.full |= (addr & 0xFFFFFFFFFFFF);
    }
    code.push_back(b);
}

void masm::CodeGen::excg(NodeExcg *n, size_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->r1;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= n->r2;
    code.push_back(b);
}

void masm::CodeGen::gen()
{
    for (auto &node : *nodes)
    {
        switch (node.kind)
        {
        case NOP:
            generate_singles(OP_NOP);
            break;
        case HLT:
            generate_singles(OP_HALT);
            break;
        case ADD_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_ADD_IMM);
            break;
        case ADD_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_ADD_REG);
            break;
        case ADD_MEM:
            arithmetic_inst_mem(GET(NodeArithmetic), OP_ADD_MEMB);
            break;
        case SUB_MEM:
            arithmetic_inst_mem(GET(NodeArithmetic), OP_SUB_MEMB);
            break;
        case MUL_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_MUL_IMM);
            break;
        case MUL_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_MUL_REG);
            break;
        case MUL_MEM:
            arithmetic_inst_mem(GET(NodeArithmetic), OP_MUL_MEMB);
            break;
        case DIV_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_DIV_IMM);
            break;
        case DIV_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_DIV_REG);
            break;
        case DIV_MEM:
            arithmetic_inst_mem(GET(NodeArithmetic), OP_DIV_MEMB);
            break;
        case MOD_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_MOD_IMM);
            break;
        case MOD_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_MOD_REG);
            break;
        case MOD_MEM:
            arithmetic_inst_mem(GET(NodeArithmetic), OP_MOD_MEMB);
            break;
        case IADD_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_IADD_IMM);
            break;
        case IADD_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_IADD_REG);
            break;
        case ISUB_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_ISUB_IMM);
            break;
        case ISUB_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_ISUB_REG);
            break;
        case IMUL_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_IMUL_IMM);
            break;
        case IMUL_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_IMUL_REG);
            break;
        case IDIV_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_IDIV_IMM);
            break;
        case IDIV_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_IDIV_REG);
            break;
        case IMOD_IMM:
            arithmetic_inst_imm(GET(NodeArithmetic), OP_IMOD_IMM);
            break;
        case IMOD_REG:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_IMOD_REG);
            break;
        case FADD:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FADD32);
            break;
        case FSUB:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FSUB32);
            break;
        case FMUL:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FMUL32);
            break;
        case FDIV:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FDIV32);
            break;
        case LFADD:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FADD);
            break;
        case LFSUB:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FSUB);
            break;
        case LFMUL:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FMUL);
            break;
        case LFDIV:
            arithmetic_inst_reg(GET(NodeArithmetic), OP_FDIV);
            break;
        case FADD_MEM:
            float_mem(GET(NodeArithmetic), OP_FADD32_MEM);
            break;
        case FSUB_MEM:
            float_mem(GET(NodeArithmetic), OP_FSUB32_MEM);
            break;
        case FMUL_MEM:
            float_mem(GET(NodeArithmetic), OP_FMUL32_MEM);
            break;
        case FDIV_MEM:
            float_mem(GET(NodeArithmetic), OP_FDIV32_MEM);
            break;
        case LFADD_MEM:
            float_mem(GET(NodeArithmetic), OP_FADD_MEM);
            break;
        case LFSUB_MEM:
            float_mem(GET(NodeArithmetic), OP_FSUB_MEM);
            break;
        case LFMUL_MEM:
            float_mem(GET(NodeArithmetic), OP_FMUL_MEM);
            break;
        case LFDIV_MEM:
            float_mem(GET(NodeArithmetic), OP_FDIV_MEM);
            break;
        case MOV_IMM:
            mov_imm(GET(NodeMov), false);
            break;
        case MOV_REG:
        case MOVL_REG:
            mov_reg(GET(NodeMov), OP_MOVE_REG);
            break;
        case MOV_VAR:
        case MOVL_VAR:
            mov_var(GET(NodeMov));
            break;
        case MOVL_IMM:
            mov_imm(GET(NodeMov), true);
            break;
        case MOVB:
            mov_reg(GET(NodeMov), OP_MOVE_REG8);
            break;
        case MOVW:
            mov_reg(GET(NodeMov), OP_MOVE_REG16);
            break;
        case MOVD:
            mov_reg(GET(NodeMov), OP_MOVE_REG32);
            break;
        case MOVSXB_IMM:
            movsx_imm(GET(NodeMov), OP_MOVESX_IMM8);
            break;
        case MOVSXB_REG:
            mov_reg(GET(NodeMov), OP_MOVESX_REG8);
            break;
        case MOVSXW_IMM:
            movsx_imm(GET(NodeMov), OP_MOVESX_IMM16);
            break;
        case MOVSXW_REG:
            mov_reg(GET(NodeMov), OP_MOVESX_REG16);
            break;
        case MOVSXD_IMM:
            movsx_imm(GET(NodeMov), OP_MOVESX_IMM32);
            break;
        case MOVSXD_REG:
            mov_reg(GET(NodeMov), OP_MOVESX_REG32);
            break;
        case JMP:
            branch(GET(NodeName), OP_JMP_ADDR);
            break;
        case CALL:
            branch(GET(NodeName), OP_CALL);
            break;
        case CALL_REG:
            branch(GET(NodeName), OP_CALL_REG);
            break;
        case RET:
            generate_singles(OP_RET);
            break;
        case SVA_IMM:
            sva_svc(GET(NodeStack), OP_SVA);
            break;
        case SVA_REG:
            sva_svc(GET(NodeStack), OP_SVA_REG);
            break;
        case SVA_VAR:
            sva_svc(GET(NodeStack), OP_SVA_MEM);
            break;
        case SVC_IMM:
            sva_svc(GET(NodeStack), OP_SVC);
            break;
        case SVC_REG:
            sva_svc(GET(NodeStack), OP_SVC_REG);
            break;
        case SVC_VAR:
            sva_svc(GET(NodeStack), OP_SVC_MEM);
            break;
        case PUSHA:
            generate_singles(OP_PUSHA);
            break;
        case POPA:
            generate_singles(OP_POPA);
            break;
        case PUSH_IMM:
            stack(GET(NodeStack), OP_PUSH_IMM);
            break;
        case PUSH_REG:
            stack(GET(NodeStack), OP_PUSH_REG);
            break;
        case PUSH_VAR:
            stack(GET(NodeStack), OP_PUSH_MEMB);
            break;
        case POP_REG:
            stack(GET(NodeStack), OP_POP);
            break;
        case POP_VAR:
            stack(GET(NodeStack), OP_POP_MEMB);
            break;
        case NOT:
            logical_singles(GET(NodeName), OP_NOT);
            break;
        case INC:
            logical_singles(GET(NodeName), OP_INC);
            break;
        case DEC:
            logical_singles(GET(NodeName), OP_DEC);
            break;
        case AND_IMM:
            logical_inst_imm(GET(NodeLogical), OP_AND_IMM);
            break;
        case AND_REG:
            logical_inst_reg(GET(NodeLogical), OP_AND_REG);
            break;
        case OR_IMM:
            logical_inst_imm(GET(NodeLogical), OP_OR_IMM);
            break;
        case OR_REG:
            logical_inst_reg(GET(NodeLogical), OP_OR_REG);
            break;
        case XOR_IMM:
            logical_inst_imm(GET(NodeLogical), OP_XOR_IMM);
            break;
        case XOR_REG:
            logical_inst_reg(GET(NodeLogical), OP_XOR_REG);
            break;
        case LSHIFT:
            logical_inst_imm(GET(NodeLogical), OP_LSHIFT);
            break;
        case RSHIFT:
            logical_inst_imm(GET(NodeLogical), OP_RSHIFT);
            break;
        case CMP_IMM:
            logical_inst_imm(GET(NodeLogical), OP_CMP_IMM);
            break;
        case CMP_REG:
            logical_inst_reg(GET(NodeLogical), OP_CMP_REG);
            break;
        case CMP_VAR:
            cmp_var(GET(NodeLogical));
            break;
        case LEA:
            lea(GET(NodeLea));
            break;
        case LOADB_REG:
            mov_reg(GET(NodeLoadStore), OP_LOADB_REG);
            break;
        case LOADW_REG:
            mov_reg(GET(NodeLoadStore), OP_LOADW_REG);
            break;
        case LOADD_REG:
            mov_reg(GET(NodeLoadStore), OP_LOADD_REG);
            break;
        case LOADQ_REG:
            mov_reg(GET(NodeLoadStore), OP_LOAD_REG);
            break;
        case LOADB_VAR:
            load_store_var(GET(NodeLoadStore), OP_LOADB);
            break;
        case LOADW_VAR:
            load_store_var(GET(NodeLoadStore), OP_LOADW);
            break;
        case LOADD_VAR:
            load_store_var(GET(NodeLoadStore), OP_LOADD);
            break;
        case LOADQ_VAR:
            load_store_var(GET(NodeLoadStore), OP_LOAD);
            break;
        case ALOADB_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_LOADB);
            break;
        case ALOADW_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_LOADW);
            break;
        case ALOADD_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_LOADD);
            break;
        case ALOADQ_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_LOAD);
            break;
        case STOREB_REG:
            mov_reg(GET(NodeLoadStore), OP_STOREB_REG);
            break;
        case STOREW_REG:
            mov_reg(GET(NodeLoadStore), OP_STOREW_REG);
            break;
        case STORED_REG:
            mov_reg(GET(NodeLoadStore), OP_STORED_REG);
            break;
        case STOREQ_REG:
            mov_reg(GET(NodeLoadStore), OP_STORE_REG);
            break;
        case STOREB_VAR:
            load_store_var(GET(NodeLoadStore), OP_STOREB);
            break;
        case STOREW_VAR:
            load_store_var(GET(NodeLoadStore), OP_STOREW);
            break;
        case STORED_VAR:
            load_store_var(GET(NodeLoadStore), OP_STORED);
            break;
        case STOREQ_VAR:
            load_store_var(GET(NodeLoadStore), OP_STORE);
            break;
        case ASTOREB_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_STOREB);
            break;
        case ASTOREW_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_STOREW);
            break;
        case ASTORED_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_STORED);
            break;
        case ASTOREQ_VAR:
            load_store_var(GET(NodeLoadStore), OP_ATOMIC_STORE);
            break;
        case CMPXCHG:
            cmpxchg(GET(NodeCmpxchg), false);
            break;
        case CMPXCHG_REGR:
            cmpxchg(GET(NodeCmpxchg), true);
            break;
        case OUTR:
            generate_singles(OP_OUTR);
            break;
        case UOUTR:
            generate_singles(OP_UOUTR);
            break;
        case CIN:
            generate_singles(OP_CIN);
            break;
        case COUT:
            generate_singles(OP_COUT);
            break;
        case SIN:
        {
            GenBinary b;
            b.bytes.b1 = OP_SIN;
            size_t addr = (*data_addr)[std::get<std::string>((GET(NodeIO))->oper)];
            b.full |= (addr & 0xFFFFFFFFFFFF);
            code.push_back(b);
            break;
        }
        case SIN_REGR:
            generate_singles(OP_SIN_REG);
            break;
        case SOUT:
        {
            GenBinary b;
            b.bytes.b1 = OP_SOUT;
            size_t addr = (*data_addr)[std::get<std::string>((GET(NodeIO))->oper)];
            b.full |= (addr & 0xFFFFFFFFFFFF);
            code.push_back(b);
            break;
        }
        case SOUT_REGR:
            generate_singles(OP_SOUT_REG);
            break;
        case IN:
            generate_singles(OP_IN);
            break;
        case INW:
            generate_singles(OP_INW);
            break;
        case IND:
            generate_singles(OP_IND);
            break;
        case INQ:
            generate_singles(OP_INQ);
            break;
        case UIN:
            generate_singles(OP_UIN);
            break;
        case UINW:
            generate_singles(OP_UINW);
            break;
        case UIND:
            generate_singles(OP_UIND);
            break;
        case UINQ:
            generate_singles(OP_UINQ);
            break;
        case OUT:
            generate_singles(OP_OUT);
            break;
        case OUTW:
            generate_singles(OP_OUTW);
            break;
        case OUTD:
            generate_singles(OP_OUTD);
            break;
        case OUTQ:
            generate_singles(OP_OUTQ);
            break;
        case UOUT:
            generate_singles(OP_UOUT);
            break;
        case UOUTW:
            generate_singles(OP_UOUTW);
            break;
        case UOUTD:
            generate_singles(OP_UOUTD);
            break;
        case UOUTQ:
            generate_singles(OP_UOUTQ);
            break;
        case INF:
            generate_singles(OP_INF32);
            break;
        case INLF:
            generate_singles(OP_INF);
            break;
        case OUTF:
            generate_singles(OP_OUTF32);
            break;
        case OUTLF:
            generate_singles(OP_OUTF);
            break;
        case EXCGB:
            excg(GET(NodeExcg), OP_EXCG8);
            break;
        case EXCGW:
            excg(GET(NodeExcg), OP_EXCG16);
            break;
        case EXCGD:
            excg(GET(NodeExcg), OP_EXCG32);
            break;
        case EXCGQ:
            excg(GET(NodeExcg), OP_EXCG);
            break;
        case MOVEB:
            mov_reg(GET(NodeMov), OP_MOV8);
            break;
        case MOVEW:
            mov_reg(GET(NodeMov), OP_MOV16);
            break;
        case MOVED:
            mov_reg(GET(NodeMov), OP_MOV32);
            break;
        case CFLAGS:
            generate_singles(OP_CFLAGS);
            break;
        case NRESET:
            generate_singles(OP_RESET);
            break;
        case CLZ:
            generate_singles(OP_CLZ);
            break;
        case CLN:
            generate_singles(OP_CLN);
            break;
        case CLC:
            generate_singles(OP_CLC);
            break;
        case CLO:
            generate_singles(OP_CLO);
            break;
        case JNZ:
            branch(GET(NodeName), OP_JNZ);
            break;
        case JZ:
            branch(GET(NodeName), OP_JZ);
            break;
        case JNE:
            branch(GET(NodeName), OP_JNE);
            break;
        case JE:
            branch(GET(NodeName), OP_JE);
            break;
        case JNC:
            branch(GET(NodeName), OP_JNC);
            break;
        case JC:
            branch(GET(NodeName), OP_JC);
            break;
        case JNO:
            branch(GET(NodeName), OP_JNO);
            break;
        case JO:
            branch(GET(NodeName), OP_JO);
            break;
        case JNN:
            branch(GET(NodeName), OP_JNN);
            break;
        case JN:
            branch(GET(NodeName), OP_JN);
            break;
        case JNG:
            branch(GET(NodeName), OP_JNG);
            break;
        case JG:
            branch(GET(NodeName), OP_JG);
            break;
        case JNS:
            branch(GET(NodeName), OP_JNS);
            break;
        case JS:
            branch(GET(NodeName), OP_JS);
            break;
        case JGE:
            branch(GET(NodeName), OP_JGE);
            break;
        case JSE:
            branch(GET(NodeName), OP_JSE);
            break;
        case LOOP:
            branch(GET(NodeName), OP_LOOP);
            break;
        case INTR:
        {
            GenBinary b;
            b.bytes.b1 = OP_INTR;
            b.full |= std::stoull(std::get<std::string>((GET(NodeIntr))->val));
            code.push_back(b);
            break;
        }
        case SETE:
            branch(GET(NodeName), OP_SET_EXCP);
            break;
        case CALLE:
            generate_singles(OP_CALL_EXCP);
            break;
        case SYSCALL:
            generate_singles(OP_SYSCALL);
            break;
        }
    }
}

void masm::CodeGen::gen_ST()
{
    size_t i = 0;
    for (auto l : *lbl_addr)
    {
        symd[l.second] = i;
        for (char _c : l.first)
        {
            ST.push_back(_c);
            i++;
        }
        ST.push_back(0);
        i++;
    }
    for (auto l : *data_addr)
    {
        symd[l.second] = i;
        for (char _c : l.first)
        {
            ST.push_back(_c);
            i++;
        }
        ST.push_back(0);
        i++;
    }
    while ((ST.size() % 8) != 0)
    {
        ST.push_back(0);
    }
}

void masm::CodeGen::setup_emit(Emit *emit)
{
   emit->code = &code;
   emit->data = data;
   emit->lbl_addr = lbl_addr;
   emit->ST = &ST;
   emit->str_data = str;
   emit->symd = &symd;
} 