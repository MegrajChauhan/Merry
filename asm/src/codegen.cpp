#include "codegen.hpp"

void masm::CodeGen::setup_codegen(SymbolTable *_t, std::vector<Node> *_n)
{
    nodes = _n;
    table = _t;
}

/**
 * A very tough choice:
 * Either I add the logic for every instruction in the same function or I make separate functions that will do the same for a group of similar instruction.
 * The problems: The first approach will offend pro C++ devs or any programmer in general(Even I would be pissed looking at a function that is over a 1000 lines long)
 * There will be an insane amount of redundant code but on the +ve side, things will be very easy and simple.
 * The second approach really has just one problem, it is complicated to implement the specific functions for each group.
 * I have to make the grouping even simpler i.e one group may have just one or two things in common at most which leaves us with at most 2 instructions per group.
 * That isn't a good enough reason to go through writing complex function logic.
 * Thus there will be a hybrid approach. The groups that will help reduce the function size will have a separate function and no otherwise.
 * Even with that, the function should get well over 600 lines of code.
 */

void masm::CodeGen::handle_arithmetic_reg_imm(msize_t op, NodeArithmetic *a)
{
    GenBinary b;
    b.bytes.b1 = op; // the opcode
    // 0xFFFFFFFF
    b.bytes.b2 = a->reg;
    b.full |= std::stoull(std::get<std::string>(a->second_oper)) & 0xFFFFFFFF;
    code.push_back(b);
}

void masm::CodeGen::handle_arithmetic_reg_reg(msize_t op, NodeArithmetic *a)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = a->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(a->second_oper);
    code.push_back(b);
}

void masm::CodeGen::handle_arithmetic_reg_var(NodeArithmetic *a, msize_t op)
{
    GenBinary b;
    std::string var = std::get<std::string>(a->second_oper);
    size_t var_addr = data_addr[std::get<std::string>(a->second_oper)];
    auto var_dets = table->variables[table->_var_list.find(var)->second];
    b.bytes.b8 = a->reg;
    switch (var_dets.type)
    {
    case BYTE:
        b.bytes.b1 = op;
        break;
    case WORD:
        b.bytes.b1 = (op + 1);
        break;
    case DWORD:
    case FLOAT:
        b.bytes.b1 = (op + 2);
        break;
    case QWORD:
    case LFLOAT:
        b.bytes.b1 = (op + 3);
        break;
    }
    b.full |= (var_addr & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

bool masm::CodeGen::generate()
{
    // first generate for the data
    generate_data();
    give_address_to_labels();
    for (auto &node : *nodes)
    {
        switch (node.kind)
        {
        case NOP:
        {
            GenBinary b;
            b.full = 0;
            code.push_back(b);
            break;
        }
        case HLT:
        {
            GenBinary b;
            b.bytes.b1 = OP_HALT;
            code.push_back(b);
            break;
        }
        case RET:
        {
            GenBinary b;
            b.bytes.b1 = OP_RET;
            code.push_back(b);
            break;
        }
        case ADD_IMM:
            handle_arithmetic_reg_imm(OP_ADD_IMM, (NodeArithmetic *)node.node.get());
            break;
        case SUB_IMM:
            handle_arithmetic_reg_imm(OP_SUB_IMM, (NodeArithmetic *)node.node.get());
            break;
        case MUL_IMM:
            handle_arithmetic_reg_imm(OP_MUL_IMM, (NodeArithmetic *)node.node.get());
            break;
        case DIV_IMM:
            handle_arithmetic_reg_imm(OP_DIV_IMM, (NodeArithmetic *)node.node.get());
            break;
        case MOD_IMM:
            handle_arithmetic_reg_imm(OP_MOD_IMM, (NodeArithmetic *)node.node.get());
            break;
        case ADD_REG:
            handle_arithmetic_reg_reg(OP_ADD_REG, (NodeArithmetic *)node.node.get());
            break;
        case SUB_REG:
            handle_arithmetic_reg_reg(OP_SUB_REG, (NodeArithmetic *)node.node.get());
            break;
        case MUL_REG:
            handle_arithmetic_reg_reg(OP_MUL_REG, (NodeArithmetic *)node.node.get());
            break;
        case DIV_REG:
            handle_arithmetic_reg_reg(OP_DIV_REG, (NodeArithmetic *)node.node.get());
            break;
        case MOD_REG:
            handle_arithmetic_reg_reg(OP_MOD_REG, (NodeArithmetic *)node.node.get());
            break;
        case ADD_MEM:
            handle_arithmetic_reg_var((NodeArithmetic *)node.node.get(), OP_ADD_MEMB);
            break;
        case SUB_MEM:
            handle_arithmetic_reg_var((NodeArithmetic *)node.node.get(), OP_SUB_MEMB);
            break;
        case MUL_MEM:
            handle_arithmetic_reg_var((NodeArithmetic *)node.node.get(), OP_MUL_MEMB);
            break;
        case DIV_MEM:
            handle_arithmetic_reg_var((NodeArithmetic *)node.node.get(), OP_DIV_MEMB);
            break;
        case MOD_MEM:
            handle_arithmetic_reg_var((NodeArithmetic *)node.node.get(), OP_MOD_MEMB);
            break;
        case FADD:
            handle_arithmetic_reg_reg(OP_FADD32, (NodeArithmetic *)node.node.get());
            break;
        case LFADD:
            handle_arithmetic_reg_reg(OP_FADD, (NodeArithmetic *)node.node.get());
            break;
        case FSUB:
            handle_arithmetic_reg_reg(OP_FSUB32, (NodeArithmetic *)node.node.get());
            break;
        case LFSUB:
            handle_arithmetic_reg_reg(OP_FSUB, (NodeArithmetic *)node.node.get());
            break;
        case FMUL:
            handle_arithmetic_reg_reg(OP_FMUL32, (NodeArithmetic *)node.node.get());
            break;
        case LFMUL:
            handle_arithmetic_reg_reg(OP_FMUL, (NodeArithmetic *)node.node.get());
            break;
        case FDIV:
            handle_arithmetic_reg_reg(OP_FDIV32, (NodeArithmetic *)node.node.get());
            break;
        case LFDIV:
            handle_arithmetic_reg_reg(OP_FDIV, (NodeArithmetic *)node.node.get());
            break;
        case MOV_IMM:
            handle_mov_reg_imm(false, (NodeMov *)node.node.get());
            break;
        case MOV_REG:
        case MOVL_REG:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVE_REG);
            break;
        case MOV_VAR:
        case MOVL_VAR:
            handle_mov_reg_var((NodeMov *)node.node.get());
            break;
        case MOVL_IMM:
            handle_mov_reg_imm(true, (NodeMov *)node.node.get());
            break;
        case MOVB:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVE_REG8);
            break;
        case MOVW:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVE_REG16);
            break;
        case MOVD:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVE_REG32);
            break;
        case MOVSXB_IMM:
        {
            NodeArithmetic a;
            auto t = (NodeMov *)node.node.get();
            a.reg = t->reg;
            a.second_oper = std::get<std::pair<std::string, DataType>>(t->second_oper).first;
            handle_arithmetic_reg_imm(OP_MOVESX_IMM8, &a);
            break;
        }
        case MOVSXW_IMM:
        {
            NodeArithmetic a;
            auto t = (NodeMov *)node.node.get();
            a.reg = t->reg;
            a.second_oper = std::get<std::pair<std::string, DataType>>(t->second_oper).first;
            handle_arithmetic_reg_imm(OP_MOVESX_IMM16, &a);
            break;
        }
        case MOVSXD_IMM:
        {
            NodeArithmetic a;
            auto t = (NodeMov *)node.node.get();
            a.reg = t->reg;
            a.second_oper = std::get<std::pair<std::string, DataType>>(t->second_oper).first;
            handle_arithmetic_reg_imm(OP_MOVESX_IMM32, &a);
            break;
        }
        case MOVSXB_REG:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVESX_REG8);
            break;
        case MOVSXW_REG:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVESX_REG16);
            break;
        case MOVSXD_REG:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOVESX_REG32);
            break;
        case MOVSXB_VAR:
        {
            // resuing a function
            auto v = (NodeMov *)node.node.get();
            handle_mov_reg_var(v);
            // add a second instruction
            GenBinary b;
            b.bytes.b1 = OP_MOVESX_REG8;
            b.bytes.b8 = v->reg;
            b.bytes.b8 <<= 4;
            b.bytes.b8 |= v->reg;
            code.push_back(b);
            break;
        }
        case MOVSXW_VAR:
        {
            auto v = (NodeMov *)node.node.get();
            handle_mov_reg_var(v);
            GenBinary b;
            b.bytes.b1 = OP_MOVESX_REG16;
            b.bytes.b8 = v->reg;
            b.bytes.b8 <<= 4;
            b.bytes.b8 |= v->reg;
            code.push_back(b);
            break;
        }
        case MOVSXD_VAR:
        {
            auto v = (NodeMov *)node.node.get();
            handle_mov_reg_var(v);
            GenBinary b;
            b.bytes.b1 = OP_MOVESX_REG32;
            b.bytes.b8 = v->reg;
            b.bytes.b8 <<= 4;
            b.bytes.b8 |= v->reg;
            code.push_back(b);
            break;
        }
        case JMP:
            handle_jmp(OP_JMP_ADDR, (NodeName *)node.node.get());
            break;
        case CALL:
        {
            NodeCall *c = (NodeCall *)node.node.get();
            GenBinary b;
            b.bytes.b1 = OP_CALL;
            b.full |= (label_addr[std::get<std::string>(c->_oper)] & 0xFFFFFFFFFFFF);
            code.push_back(b);
            break;
        }
        case CALL_REG:
        {
            NodeCall *c = (NodeCall *)node.node.get();
            GenBinary b;
            b.bytes.b1 = OP_CALL_REG;
            b.full |= (std::get<Register>(c->_oper));
            code.push_back(b);
            break;
        }
        case SVA_REG:
            handle_mov_reg_reg((NodeSTACK *)node.node.get(), OP_SVA_REG);
            break;
        case SVC_REG:
            handle_mov_reg_reg((NodeSTACK *)node.node.get(), OP_SVC_REG);
            break;
        case SVC_IMM:
            handle_mov_reg_imm(false, (NodeSTACK *)node.node.get());
            code[code.size() - 1].bytes.b1 = OP_SVC;
            break;
        case SVA_IMM:
            handle_mov_reg_imm(false, (NodeSTACK *)node.node.get());
            code[code.size() - 1].bytes.b1 = OP_SVA;
            break;
        case SVA_VAR:
        {
            GenBinary b;
            auto n = (NodeSTACK *)node.node.get();
            b.bytes.b1 = OP_SVA_MEM;
            b.bytes.b2 = n->reg;
            b.full |= data_addr[std::get<std::pair<std::string, DataType>>(n->second_oper).first];
            code.push_back(b);
            break;
        }
        case SVC_VAR:
        {
            GenBinary b;
            auto n = (NodeSTACK *)node.node.get();
            b.bytes.b1 = OP_SVC_MEM;
            b.bytes.b2 = n->reg;
            b.full |= data_addr[std::get<std::pair<std::string, DataType>>(n->second_oper).first];
            code.push_back(b);
            break;
        }
        case PUSHA:
        {
            GenBinary b;
            b.bytes.b1 = OP_PUSHA;
            code.push_back(b);
            break;
        }
        case POPA:
        {
            GenBinary b;
            b.bytes.b1 = OP_POPA;
            code.push_back(b);
            break;
        }
        case PUSH_REG:
            handle_push_pop_reg(OP_PUSH_REG, (NodePushPop *)node.node.get());
            break;
        case POP_REG:
            handle_push_pop_reg(OP_POP, (NodePushPop *)node.node.get());
            break;
        case PUSH_IMM:
        case POP_IMM: // just to keep the joke alive and stop compiler warnings obviously
            handle_push_imm((NodePushPop *)node.node.get());
            break;
        case PUSH_VAR:
            handle_push_pop_var(OP_PUSH_MEMB, (NodePushPop *)node.node.get());
            break;
        case POP_VAR:
            handle_push_pop_var(OP_POP_MEMB, (NodePushPop *)node.node.get());
            break;
        case NOT:
            handle_single_regr(OP_NOT, (NodeSingleRegr *)node.node.get());
            break;
        case INC:
            handle_single_regr(OP_INC, (NodeSingleRegr *)node.node.get());
            break;
        case DEC:
            handle_single_regr(OP_DEC, (NodeSingleRegr *)node.node.get());
            break;
        case AND_IMM:
            handle_logical_reg_imm(OP_AND_IMM, (NodeLogical *)node.node.get());
            break;
        case OR_IMM:
            handle_logical_reg_imm(OP_OR_IMM, (NodeLogical *)node.node.get());
            break;
        case XOR_IMM:
            handle_logical_reg_imm(OP_XOR_IMM, (NodeLogical *)node.node.get());
            break;
        case LSHIFT:
            handle_logical_reg_imm(OP_LSHIFT, (NodeLogical *)node.node.get());
            break;
        case RSHIFT:
            handle_logical_reg_imm(OP_RSHIFT, (NodeLogical *)node.node.get());
            break;
        case CMP_IMM:
            handle_logical_reg_imm(OP_CMP_IMM, (NodeLogical *)node.node.get());
            break;
        case AND_REG:
            handle_logical_reg_reg(OP_AND_REG, (NodeLogical *)node.node.get());
            break;
        case OR_REG:
            handle_logical_reg_reg(OP_OR_REG, (NodeLogical *)node.node.get());
            break;
        case XOR_REG:
            handle_logical_reg_reg(OP_XOR_REG, (NodeLogical *)node.node.get());
            break;
        case CMP_REG:
            handle_logical_reg_reg(OP_CMP_REG, (NodeLogical *)node.node.get());
            break;
        case CMP_VAR:
            handle_arithmetic_reg_var((NodeLogical *)node.node.get(), OP_CMP_IMM_MEMB);
            break;
        case LEA:
            handle_lea((NodeLea *)node.node.get());
            break;
        case LOADB_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_LOADB);
            break;
        case ALOADB_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_LOADB);
            break;
        case STOREB_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_STOREB);
            break;
        case ASTOREB_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_STOREB);
            break;
        case LOADW_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_LOADW);
            break;
        case ALOADW_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_LOADW);
            break;
        case STOREW_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_STOREW);
            break;
        case ASTOREW_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_STOREW);
            break;
        case LOADD_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_LOADD);
            break;
        case ALOADD_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_LOADD);
            break;
        case STORED_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_STORED);
            break;
        case ASTORED_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_STORED);
            break;
        case LOADQ_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_LOAD);
            break;
        case ALOADQ_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_LOAD);
            break;
        case STOREQ_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_STORE);
            break;
        case ASTOREQ_VAR:
            handle_load_store_reg_var((NodeLoadStore *)node.node.get(), OP_ATOMIC_STORE);
            break;
        case LOADB_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_LOADB_REG);
            break;
        case LOADW_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_LOADW_REG);
            break;
        case LOADD_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_LOADD_REG);
            break;
        case LOADQ_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_LOAD_REG);
            break;
        case STOREB_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_STOREB_REG);
            break;
        case STOREW_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_STOREW_REG);
            break;
        case STORED_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_STORED_REG);
            break;
        case STOREQ_REG:
            handle_load_store_reg_reg((NodeLoadStore *)node.node.get(), OP_STORE_REG);
            break;
        }
    }
    for (auto b : code)
    {
        printf("%lX\n", b.full);
    }
    printf("\n");
    for (auto b : data)
    {
        std::cout << ((int)b) << "\n";
    }
    for (auto b : str_data)
    {
        std::cout << ((int)b) << "\n";
    }
    printf("\n");
    for (auto l : label_addr)
    {
        printf("%s: %lX\n", l.first.c_str(), l.second);
    }
    printf("\n");
    for (auto l : data_addr)
    {
        printf("%s: %lX\n", l.first.c_str(), l.second);
    }
    return true;
}

void masm::CodeGen::generate_data()
{
    size_t start = 0;
    for (auto var : table->variables)
    {
        switch (var.type)
        {
        case BYTE:
        {
            data_addr[var.name] = start;
            data.push_back(std::stoull(var.value) & 0xFF);
            start++;
            break;
        }
        case WORD:
        {
            data_addr[var.name] = start;
            mword_t v = std::stoull(var.value);
            data.push_back(v & 0xFF);
            data.push_back((v >> 8) & 0xFF);
            start += 2;
            break;
        }
        case DWORD:
        {
            data_addr[var.name] = start;
            mdword_t v = std::stoull(var.value);
            data.push_back(v & 0xFF);
            data.push_back((v >> 8) & 0xFF);
            data.push_back((v >> 16) & 0xFF);
            data.push_back((v >> 24) & 0xFF);
            start += 4;
            break;
        }
        case QWORD:
        {
            data_addr[var.name] = start;
            mqword_t v = std::stoull(var.value);
            data.push_back(v & 0xFF);
            data.push_back((v >> 8) & 0xFF);
            data.push_back((v >> 16) & 0xFF);
            data.push_back((v >> 24) & 0xFF);
            data.push_back((v >> 32) & 0xFF);
            data.push_back((v >> 40) & 0xFF);
            data.push_back((v >> 48) & 0xFF);
            data.push_back((v >> 56) & 0xFF);
            start += 8;
            break;
        }
        case FLOAT:
        {
            data_addr[var.name] = start;
            mdword_t v = (mdword_t)(std::stof(var.value));
            data.push_back(v & 0xFF);
            data.push_back((v >> 8) & 0xFF);
            data.push_back((v >> 16) & 0xFF);
            data.push_back((v >> 24) & 0xFF);
            start += 4;
            break;
        }
        case LFLOAT:
        {
            data_addr[var.name] = start;
            mqword_t v = (mqword_t)(std::stod(var.value));
            data.push_back(v & 0xFF);
            data.push_back((v >> 8) & 0xFF);
            data.push_back((v >> 16) & 0xFF);
            data.push_back((v >> 24) & 0xFF);
            data.push_back((v >> 32) & 0xFF);
            data.push_back((v >> 40) & 0xFF);
            data.push_back((v >> 48) & 0xFF);
            data.push_back((v >> 56) & 0xFF);
            start += 8;
            break;
        }
        case RESB:
        {
            data_addr[var.name] = start;
            size_t len = std::stoull(var.value);
            for (size_t i = 0; i < len; i++)
                data.push_back(0);
            start += len;
            break;
        }
        case RESW:
        {
            data_addr[var.name] = start;
            size_t len = std::stoull(var.value) * 2;
            for (size_t i = 0; i < len; i++)
                data.push_back(0);
            start += len;
            break;
        }
        case RESD:
        {
            data_addr[var.name] = start;
            size_t len = std::stoull(var.value) * 4;
            for (size_t i = 0; i < len; i++)
                data.push_back(0);
            start += len;
            break;
        }
        case RESQ:
        {
            data_addr[var.name] = start;
            size_t len = std::stoull(var.value) * 8;
            for (size_t i = 0; i < len; i++)
                data.push_back(0);
            start += len;
            break;
        }
        }
    }
    for (auto var : table->variables)
    {
        switch (var.type)
        {
        case STRING:
        {
            data_addr[var.name] = start;
            for (auto _c : var.value)
            {
                str_data.push_back(_c);
            }
            start += var.value.length();
            break;
        }
        }
    }
}

void masm::CodeGen::handle_mov_reg_imm(bool l, NodeMov *n)
{
    GenBinary b;
    b.bytes.b1 = (l) ? OP_MOVE_IMM_64 : OP_MOVE_IMM;
    if (l)
    {
        b.bytes.b8 = n->reg;
        code.push_back(b);
        b.full = 0;
    }
    auto second = std::get<std::pair<std::string, DataType>>(n->second_oper);
    switch (second.second)
    {
    case BYTE:
    {
        b.full |= std::stoull(second.first) & (l ? 0xFFFFFFFFFFFFFFFF : 0xFFFFFFFF);
        break;
    }
    case FLOAT:
    {
        b.full |= ((mdword_t)std::stof(second.first)) & (l ? 0xFFFFFFFFFFFFFFFF : 0xFFFFFFFF);
        break;
    }
    }
    code.push_back(b);
}

void masm::CodeGen::handle_mov_reg_reg(NodeMov *n, msize_t op)
{
    // written with idea of potential expansion for other instructions as well.
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(n->second_oper);
    code.push_back(b);
}

void masm::CodeGen::handle_mov_reg_var(NodeMov *n)
{
    GenBinary b;
    auto _s = std::get<std::pair<std::string, DataType>>(n->second_oper);
    auto _res = table->_var_list.find(_s.first);
    bool is_lbl = false;
    if (_res == table->_var_list.end())
        is_lbl = true;
    size_t addr = is_lbl ? label_addr[_s.first] : data_addr.find(_s.first)->second;
    b.bytes.b2 = n->reg;
    b.full |= addr;
    if (!is_lbl)
    {
        Variable dets = table->variables[table->_var_list[_s.first]];
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
        case RESD:
            b.bytes.b1 = OP_LOADD;
            break;
        case QWORD:
        case RESQ:
            b.bytes.b1 = OP_LOAD;
            break;
        }
    }
    else
        b.bytes.b1 = OP_MOVE_IMM;
    code.push_back(b);
}

void masm::CodeGen::handle_jmp(msize_t op, NodeName *n)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.full |= (label_addr[n->name] & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::give_address_to_labels()
{
    size_t i = 0;
    for (auto &l : *nodes)
    {
        switch (l.kind)
        {
        case LABEL:
        {
            NodeName *n = (NodeName *)l.node.get();
            label_addr[n->name] = i;
            i++;
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

void masm::CodeGen::handle_push_pop_reg(msize_t op, NodePushPop *n)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = std::get<Register>(n->val);
    code.push_back(b);
}

void masm::CodeGen::handle_push_imm(NodePushPop *n)
{
    GenBinary b;
    b.bytes.b1 = OP_PUSH_IMM;
    code.push_back(b);
    b.full = std::stoull(std::get<std::string>(n->val));
    code.push_back(b);
}

void masm::CodeGen::handle_push_pop_var(msize_t op, NodePushPop *n)
{
    GenBinary b;
    std::string name = std::get<std::string>(n->val);
    size_t addr = 0;
    if ((data_addr.find(name) == data_addr.end()))
    {
        // It must be a label
        addr = label_addr[name];
        b.bytes.b1 = OP_PUSH_IMM;
        code.push_back(b);
        b.full = addr;
    }
    else
    {
        addr = data_addr[name];
        auto dets = table->variables[table->_var_list[name]];
        switch (dets.type)
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
            b.bytes.b1 = op + 2;
            break;
        case QWORD:
        case RESQ:
            b.bytes.b1 = op + 3;
            break;
        }
        b.full |= (addr & 0xFFFFFFFFFFFF);
    }
    code.push_back(b);
}

void masm::CodeGen::handle_single_regr(msize_t op, NodeSingleRegr *n)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    code.push_back(b);
}

void masm::CodeGen::handle_logical_reg_imm(msize_t op, NodeLogical *n)
{
    GenBinary b;
    b.bytes.b1 = op;
    if (op == OP_LSHIFT || op == OP_RSHIFT)
    {
        b.bytes.b8 |= std::stoull(std::get<std::string>(n->second_oper)) & 0x40;
        b.bytes.b7 = n->reg;
    }
    else
    {
        b.bytes.b8 = n->reg;
        code.push_back(b);
        b.full = std::stoull(std::get<std::string>(n->second_oper));
    }
    code.push_back(b);
}

void masm::CodeGen::handle_logical_reg_reg(msize_t op, NodeLogical *n)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(n->second_oper);
    code.push_back(b);
}

void masm::CodeGen::handle_lea(NodeLea *n)
{
    GenBinary b;
    b.bytes.b1 = OP_LEA;
    b.bytes.b8 = n->scale;
    b.bytes.b7 = n->ind;
    b.bytes.b6 = n->base;
    b.bytes.b5 = n->dest;
    code.push_back(b);
}

void masm::CodeGen::handle_load_store_reg_var(NodeLoadStore *n, msize_t op)
{
    GenBinary b;
    Variable var = table->variables[table->_var_list[std::get<std::string>(n->second_oper)]];
    size_t addr = data_addr[std::get<std::string>(n->second_oper)];
    b.bytes.b1 = op;
    b.bytes.b2 = n->reg;
    b.full |= (addr & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::handle_load_store_reg_reg(NodeLoadStore *n, msize_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= std::get<Register>(n->second_oper);
    code.push_back(b);
}
