#include "../codegen/codegen.hpp"

masm::codegen::Codegen::Codegen(std::vector<std::unique_ptr<masm::nodes::Node>> &src)
{
    inst_nodes = std::move(src);
}

masm::codegen::Codegen::Codegen(masm::sema::Sema &sema)
{
    sema.pass_nodes(inst_nodes);
    table = sema.get_symtable();
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

void masm::codegen::Codegen::gen()
{
    gen_data(); // generate data bytes
    // now generate code based on the instructions
    for (auto &inst: inst_nodes)
    {
        switch(inst->type)
        {
            case nodes::NodeKind::_INST_HLT:
            {
                // for this instruction just push 
            }
        }
    }
}