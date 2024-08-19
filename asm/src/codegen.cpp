#include "codegen.hpp"

void masm::CodeGen::setup_codegen(SymbolTable *_t, std::vector<Node> *_n)
{
    nodes = _n;
    table = _t;
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
        case ADD_IMM:
        {
            //////
        }
        }
    }
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
            data.push_back(std::stoull(var.value) & 0xFFFF);
            start += 2;
            break;
        }
        case DWORD:
        {
            data_addr[var.name] = start;
            data.push_back(std::stoull(var.value) & 0xFFFFFFFF);
            start += 4;
            break;
        }
        case QWORD:
        {
            data_addr[var.name] = start;
            data.push_back(std::stoull(var.value));
            start += 8;
            break;
        }
        case STRING:
        {
            data_addr[var.name] = start;
            for (auto _c : var.value)
            {
                data.push_back(_c);
            }
            start += var.value.length();
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
            // for instructions that take two qwords, we need another one here
        }
    }
}