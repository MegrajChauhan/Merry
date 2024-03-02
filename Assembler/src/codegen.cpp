#include "../codegen/codegen.hpp"

masm::codegen::Codegen::Codegen(std::vector<std::unique_ptr<masm::nodes::Node>> &src)
{
    inst_nodes = std::move(src);
}

masm::codegen::Codegen::Codegen(masm::sema::Sema &sema)
{
    sema.analyse();
    sema.pass_nodes(inst_nodes);
    table = sema.get_symtable();
    main_proc_ind = sema.get_main_declr_pos();
}

void masm::codegen::Codegen::gen_data()
{
    // we need to generate addresses for all the data
    // and based on their sizes as well
    // data_addrs will hold the addresses to the first bytes of the data
    size_t start = 0; // start with addr 0
    for (auto data : table)
    {
        switch (data.second.type)
        {
        case symtable::SymEntryType::_VAR:
        {
            // don't care about procedures and labels here
            switch (data.second.dtype)
            {
            case nodes::DataType::_TYPE_BYTE:
            {
                // since the variable names are unique, we won't have to worry about anything here
                data_addrs[data.first] = start;
                start++; // just 1 byte
                data_bytes.push_back((unsigned char)std::stoi(data.second.value));
                break;
            }
            }
            break;
        }
        }
    }
}

void masm::codegen::Codegen::gen_inst_mov_reg_reg(std::unique_ptr<nodes::Node> &node)
{
    auto n = (nodes::NodeInstMovRegReg *)node->ptr.get();
    Instruction inst;
    inst.bytes.b1 = opcodes::OP_MOVE_REG;
    inst.bytes.b8 = (n->dest_regr << 4) | n->src_reg;
    inst_bytes.push_back(inst);
}

void masm::codegen::Codegen::gen_inst_mov_reg_imm(std::unique_ptr<nodes::Node> &node)
{
    // based on the situation we need to
    nodes::NodeInstMovRegImm *n = (nodes::NodeInstMovRegImm *)node->ptr.get();
    Instruction final_inst;
    if (n->is_iden)
    {
        // it is an identifier
        // get info on the identifier
        size_t addr_of_iden = data_addrs.find(n->value)->second;
        // we can be sure that it is correctly defined
        // also need to check the size of the data
        auto iden_details = table.find_entry(n->value)->second;
        switch (iden_details.dtype)
        {
        case nodes::DataType::_TYPE_BYTE:
        {
            // this is a byte type value so we need to use loadb instruction
            final_inst.bytes.b1 = opcodes::OP_LOADB;
            final_inst.bytes.b2 = (n->dest_regr);
            final_inst.whole |= addr_of_iden;
            // we then need to push the 6 byte address of the
            break;
        }
        }
    }
    else
    {
        // we just have a plain immediate
        // but we still need to know the size of the immediate
        // for moving 64-bit immdiate, we will have a dedicated instruction but the general move_imm should suffice
        final_inst.bytes.b1 = opcodes::OP_MOVE_IMM;
        final_inst.bytes.b2 = n->dest_regr;
        final_inst.whole |= std::stoi(n->value);
    }
    inst_bytes.push_back(final_inst);
}

void masm::codegen::Codegen::label_labels()
{
    // just go through each instruction
    // see if any label is there
    // then add it to the label_addrs
    size_t i = 0;
    for (auto &x : inst_nodes)
    {
        if (x->kind == nodes::NodeKind::_LABEL)
            label_addrs[((nodes::NodeLabel *)x->ptr.get())->label_name] = i;
        else if (x->kind != nodes::NodeKind::_PROC_DECLR && x->kind != nodes::NodeKind::_DEF_BYTE)
            i++;
    }
}

void masm::codegen::Codegen::gen()
{
    gen_data(); // generate data bytes
    label_labels();
    size_t count = 0; // the number of instructions; also works as an address
    std::vector<std::unique_ptr<nodes::Node>>::iterator iter = inst_nodes.begin();
    auto temp = iter;
    while (iter != inst_nodes.end())
    {
        auto inst = iter->get();
        switch (inst->kind)
        {
        case nodes::NodeKind::_INST_HLT:
        {
            Instruction inst;
            inst.bytes.b1 = opcodes::OP_HALT;
            inst_bytes.push_back(inst);
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_IMM:
        {
            // we need to change this to load instruction
            // based on the operands
            gen_inst_mov_reg_imm(*iter);
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_REG:
        {
            gen_inst_mov_reg_reg(*iter);
            break;
        }
        default:
            count--;
        }
        // we don't care about procedure declaration right now
        iter++;
        count++;
    }  
}