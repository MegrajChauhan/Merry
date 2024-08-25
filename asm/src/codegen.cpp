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
    if (data_addr.find(var) == data_addr.end())
    {
        // must be a constant
        a->second_oper = table->_const_list[var].value;
        handle_arithmetic_reg_imm(op, a);
        return;
    }
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
            handle_one(OP_NOP);
            break;
        case HLT:
            handle_one(OP_HALT);
            break;
        case RET:
            handle_one(OP_RET);
            break;
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
            handle_movsx((NodeMov *)node.node.get(), OP_MOVESX_IMM8);
            break;
        case MOVSXW_IMM:
            handle_movsx((NodeMov *)node.node.get(), OP_MOVESX_IMM16);
            break;
        case MOVSXD_IMM:
            handle_movsx((NodeMov *)node.node.get(), OP_MOVESX_IMM32);
            break;
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
            handle_movsx_var((NodeMov *)node.node.get(), OP_MOVESX_REG8);
            break;
        case MOVSXW_VAR:
            handle_movsx_var((NodeMov *)node.node.get(), OP_MOVESX_REG16);
            break;
        case MOVSXD_VAR:
            handle_movsx_var((NodeMov *)node.node.get(), OP_MOVESX_REG32);
            break;
        case JMP:
            handle_jmp(OP_JMP_ADDR, (NodeName *)node.node.get());
            break;
        case JNZ:
            handle_jmp(OP_JNZ, (NodeName *)node.node.get());
            break;
        case JZ:
            handle_jmp(OP_JZ, (NodeName *)node.node.get());
            break;
        case JNE:
            handle_jmp(OP_JNE, (NodeName *)node.node.get());
            break;
        case JE:
            handle_jmp(OP_JE, (NodeName *)node.node.get());
            break;
        case JNC:
            handle_jmp(OP_JNC, (NodeName *)node.node.get());
            break;
        case JC:
            handle_jmp(OP_JC, (NodeName *)node.node.get());
            break;
        case JNO:
            handle_jmp(OP_JNO, (NodeName *)node.node.get());
            break;
        case JO:
            handle_jmp(OP_JO, (NodeName *)node.node.get());
            break;
        case JNN:
            handle_jmp(OP_JNN, (NodeName *)node.node.get());
            break;
        case JN:
            handle_jmp(OP_JN, (NodeName *)node.node.get());
            break;
        case JNG:
            handle_jmp(OP_JNG, (NodeName *)node.node.get());
            break;
        case JG:
            handle_jmp(OP_JG, (NodeName *)node.node.get());
            break;
        case JNS:
            handle_jmp(OP_JNS, (NodeName *)node.node.get());
            break;
        case JS:
            handle_jmp(OP_JS, (NodeName *)node.node.get());
            break;
        case JSE:
            handle_jmp(OP_JSE, (NodeName *)node.node.get());
            break;
        case JGE:
            handle_jmp(OP_JE, (NodeName *)node.node.get());
            break;
        case LOOP:
            handle_jmp(OP_LOOP, (NodeName *)node.node.get());
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
            handle_sva_svc_var((NodeSTACK *)node.node.get(), OP_SVA_MEM);
            break;
        case SVC_VAR:
            handle_sva_svc_var((NodeSTACK *)node.node.get(), OP_SVC_MEM);
            break;
        case PUSHA:
            handle_one(OP_PUSHA);
            break;
        case POPA:
            handle_one(OP_POPA);
            break;
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
        case OUTR:
            handle_one(OP_OUTR);
            break;
        case UOUTR:
            handle_one(OP_UOUTR);
            break;
        case CIN:
            handle_single_regr(OP_CIN, (NodeSingleRegr *)node.node.get());
            break;
        case COUT:
            handle_single_regr(OP_COUT, (NodeSingleRegr *)node.node.get());
            break;
        case IN:
            handle_single_regr(OP_IN, (NodeSingleRegr *)node.node.get());
            break;
        case INW:
            handle_single_regr(OP_INW, (NodeSingleRegr *)node.node.get());
            break;
        case IND:
            handle_single_regr(OP_IND, (NodeSingleRegr *)node.node.get());
            break;
        case INQ:
            handle_single_regr(OP_INQ, (NodeSingleRegr *)node.node.get());
            break;
        case OUT:
            handle_single_regr(OP_OUT, (NodeSingleRegr *)node.node.get());
            break;
        case OUTW:
            handle_single_regr(OP_OUTW, (NodeSingleRegr *)node.node.get());
            break;
        case OUTD:
            handle_single_regr(OP_OUTD, (NodeSingleRegr *)node.node.get());
            break;
        case OUTQ:
            handle_single_regr(OP_OUTQ, (NodeSingleRegr *)node.node.get());
            break;
        case UOUT:
            handle_single_regr(OP_UOUT, (NodeSingleRegr *)node.node.get());
            break;
        case UOUTW:
            handle_single_regr(OP_UOUTW, (NodeSingleRegr *)node.node.get());
            break;
        case UOUTD:
            handle_single_regr(OP_UOUTD, (NodeSingleRegr *)node.node.get());
            break;
        case UOUTQ:
            handle_single_regr(OP_UOUTQ, (NodeSingleRegr *)node.node.get());
            break;
        case UIN:
            handle_single_regr(OP_UINW, (NodeSingleRegr *)node.node.get());
            break;
        case UINW:
            handle_single_regr(OP_UIN, (NodeSingleRegr *)node.node.get());
            break;
        case UIND:
            handle_single_regr(OP_UIND, (NodeSingleRegr *)node.node.get());
            break;
        case UINQ:
            handle_single_regr(OP_UINQ, (NodeSingleRegr *)node.node.get());
            break;
        case INF:
            handle_single_regr(OP_INF32, (NodeSingleRegr *)node.node.get());
            break;
        case INLF:
            handle_single_regr(OP_INF, (NodeSingleRegr *)node.node.get());
            break;
        case OUTF:
            handle_single_regr(OP_OUTF32, (NodeSingleRegr *)node.node.get());
            break;
        case OUTLF:
            handle_single_regr(OP_OUTF, (NodeSingleRegr *)node.node.get());
            break;
        case EXCGB:
            handle_excg((NodeExcg *)node.node.get(), OP_EXCG8);
            break;
        case EXCGW:
            handle_excg((NodeExcg *)node.node.get(), OP_EXCG16);
            break;
        case EXCGD:
            handle_excg((NodeExcg *)node.node.get(), OP_EXCG32);
            break;
        case EXCGQ:
            handle_excg((NodeExcg *)node.node.get(), OP_EXCG);
            break;
        case MOVEB:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOV8);
            break;
        case MOVEW:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOV16);
            break;
        case MOVED:
            handle_mov_reg_reg((NodeMov *)node.node.get(), OP_MOV32);
            break;
        case CFLAGS:
            handle_one(OP_CFLAGS);
            break;
        case NRESET:
            handle_one(OP_RESET);
            break;
        case CLZ:
            handle_one(OP_CLZ);
            break;
        case CLN:
            handle_one(OP_CLN);
            break;
        case CLC:
            handle_one(OP_CLC);
            break;
        case CLO:
            handle_one(OP_CLO);
            break;
        case INTR:
        {
            GenBinary b;
            b.bytes.b1 = OP_INTR;
            b.full |= (std::stoull(((NodeIntr *)node.node.get())->val) & 0xFFFF);
            code.push_back(b);
            break;
        }
        case FADD_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FADD32_MEM);
            break;
        case FSUB_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FSUB32_MEM);
            break;
        case FMUL_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FMUL32_MEM);
            break;
        case FDIV_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FDIV32_MEM);
            break;
        case LFADD_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FADD_MEM);
            break;
        case LFSUB_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FSUB_MEM);
            break;
        case LFMUL_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FMUL_MEM);
            break;
        case LFDIV_MEM:
            handle_float_var((NodeArithmetic *)node.node.get(), OP_FDIV_MEM);
            break;
        case CALLE:
            handle_one(OP_CALL_EXCP);
            break;
        case SETE:
            handle_jmp(OP_SET_EXCP, (NodeName *)node.node.get());
            break;
        case SYSCALL:
            handle_one(OP_SYSCALL);
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

void masm::CodeGen::handle_float_var(NodeArithmetic *n, msize_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b2 = n->reg;
    b.full |= (std::stoull(std::get<std::string>(n->second_oper)) & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::handle_movsx(NodeMov *n, msize_t op)
{
    NodeArithmetic a;
    a.reg = n->reg;
    a.second_oper = std::get<std::pair<std::string, DataType>>(n->second_oper).first;
    handle_arithmetic_reg_imm(op, &a);
}

void masm::CodeGen::handle_movsx_var(NodeMov *n, msize_t op)
{
    handle_mov_reg_var(n);
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->reg;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= n->reg;
    code.push_back(b);
}

void masm::CodeGen::handle_sva_svc_var(NodeSTACK *n, msize_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b2 = n->reg;
    auto v = std::get<std::pair<std::string, DataType>>(n->second_oper);
    if (table->_var_list.find(v.first) == table->_var_list.end())
    {
        v.first = table->_const_list[v.first].value;
        v.second = table->_const_list[v.first].type;
        handle_mov_reg_imm(false, n);
        code[code.size() - 1].bytes.b1 = op - 145;
        return;
    }
    b.full |= data_addr[std::get<std::pair<std::string, DataType>>(n->second_oper).first];
    code.push_back(b);
}

void masm::CodeGen::handle_one(msize_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    code.push_back(b);
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
    {
        if (table->_const_list.find(_s.first) == table->_const_list.end())
            is_lbl = true;
        // is a constant
        _s.first = table->_const_list[_s.first].value;
        _s.second = table->_const_list[_s.first].type;
        handle_mov_reg_imm(false, n);
        return;
    }
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

void masm::CodeGen::handle_cmpxchg(NodeCmpxchg *n)
{
    GenBinary b;
    b.bytes.b1 = OP_CMPXCHG;
    b.bytes.b2 = n->reg1;
    b.bytes.b2 <<= 4;
    b.bytes.b2 |= n->reg2;
    b.full |= (std::stoull(n->var) & 0xFFFFFFFFFFFF);
    code.push_back(b);
}

void masm::CodeGen::handle_excg(NodeExcg *n, msize_t op)
{
    GenBinary b;
    b.bytes.b1 = op;
    b.bytes.b8 = n->r1;
    b.bytes.b8 <<= 4;
    b.bytes.b8 |= n->r2;
    code.push_back(b);
}
