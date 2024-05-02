#include "../includes/sema.hpp"

masm::sema::Sema::Sema(parser::Parser &parser)
{
    parser.parse();
    // nodes = parser.get_nodes();
    filepath = parser.get_path();
    parser.move_nodes(nodes);
}

void masm::sema::Sema::set_path(std::filesystem::path path)
{
    this->filepath = path;
}

void masm::sema::Sema::analysis_error(size_t line, std::string msg)
{
    std::cerr << "While " << _CCODE_BOLD << "Analysing:\n";
    std::cerr << filepath << ": In line, " << line++ << ":" << _CCODE_RESET;
    std::cerr << " " << msg << std::endl;
    std::cerr << "Aborting further compilation." << std::endl;
    exit(EXIT_FAILURE);
}

void masm::sema::Sema::analysis_error(std::string msg)
{
    std::cerr << "While " << _CCODE_BOLD << "Analysing:\n";
    std::cerr << filepath << ":" << _CCODE_RESET;
    std::cerr << " " << msg << std::endl;
    std::cerr << "Aborting further compilation." << std::endl;
    exit(EXIT_FAILURE);
}

void masm::sema::Sema::check_proc_declr()
{
    bool found_main = false;
    for (auto x : symtable)
    {
        if (x.second.type == symtable::SymEntryType::_PROC)
        {
            if (x.second.defined != true)
                analysis_error(std::string("No definition of declared procedure '") + x.first + "' found.");
            if (x.first == "main")
                found_main = true;
        }
    }
    if (!found_main)
        analysis_error(std::string("Expected the definition for the main procedure but found none"));
}

void masm::sema::Sema::analyse()
{
    /*
     take the parsed nodes
     if the node is a variable, proc declaration or a label, create a new entry or check if the entry already exists
     put all the instruction nodes in the inst_nodes
     after finishing redo the process with the inst nodes
     check if the variables used in the instructions are valid or not
     check if the calls have the correct procedure names or not
    */
    for (auto &node : nodes)
    {
        switch (node->type)
        {
        // as there can only be two types, the other is default
        // though we also need to take care of the procedure declaration
        case nodes::NodeType::_TYPE_DATA:
        {
            // if it is a data
            // just check and push it
            switch (node->kind)
            {
            case nodes::NodeKind::_DEF_BYTE:
            {
                auto var = (nodes::NodeDefByte *)node->ptr.get();
                // check if this label already exists
                // if so then it is not allowed to redefine the same label twice
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_BYTE));
                break;
            }
            case nodes::NodeKind::_DEF_WORD:
            {
                auto var = (nodes::NodeDefWord *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_WORD));
                break;
            }
            case nodes::NodeKind::_DEF_DWORD:
            {
                auto var = (nodes::NodeDefDword *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_DWORD));
                break;
            }
            case nodes::NodeKind::_DEF_FLOAT:
            {
                auto var = (nodes::NodeDefFloat *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_FLOAT));
                break;
            }
            case nodes::NodeKind::_DEF_LFLOAT:
            {
                auto var = (nodes::NodeDefLFloat *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_LFLOAT));
                break;
            }
            case nodes::NodeKind::_DEF_QWORD:
            {
                auto var = (nodes::NodeDefQword *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_QWORD));
                break;
            }
            case nodes::NodeKind::_DEF_STRING:
            {
                auto var = (nodes::NodeDefByte *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->byte_name)))
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_STRING));
                break;
            }
            case nodes::NodeKind::_DEF_RESB:
            {
                auto var = (nodes::NodeRes *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->name)))
                    analysis_error(node->line, std::string("The variable '") + var->name + "' already exists as reserved; redefining");
                symtable.add_entry(var->name, symtable::SymTableEntry(symtable::_VAR, "", nodes::_TYPE_RESB, var->number));
                break;
            }
            case nodes::NodeKind::_DEF_RESW:
            {
                auto var = (nodes::NodeRes *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->name)))
                    analysis_error(node->line, std::string("The variable '") + var->name + "' already exists as reserved; redefining");
                symtable.add_entry(var->name, symtable::SymTableEntry(symtable::_VAR, "", nodes::_TYPE_RESW, var->number));
                break;
            }
            case nodes::NodeKind::_DEF_RESD:
            {
                auto var = (nodes::NodeRes *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->name)))
                    analysis_error(node->line, std::string("The variable '") + var->name + "' already exists as reserved; redefining");
                symtable.add_entry(var->name, symtable::SymTableEntry(symtable::_VAR, "", nodes::_TYPE_RESD, var->number));
                break;
            }
            case nodes::NodeKind::_DEF_RESQ:
            {
                auto var = (nodes::NodeRes *)node->ptr.get();
                if (symtable.is_invalid(symtable.find_entry(var->name)))
                    analysis_error(node->line, std::string("The variable '") + var->name + "' already exists as reserved; redefining");
                symtable.add_entry(var->name, symtable::SymTableEntry(symtable::_VAR, "", nodes::_TYPE_RESQ, var->number));
                break;
            }
            }
            break;
        }
        default:
            switch (node->kind)
            {
            case nodes::NodeKind::_LABEL:
            {
                auto label = (nodes::NodeLabel *)node->ptr.get();
                // check if this label already exists
                // if so then it is not allowed to redefine the same label twice
                // also check if the label is a procedure that was already defined
                auto res = symtable.find_entry(label->label_name);
                if (symtable.is_invalid(res))
                {
                    if (res->second.type == symtable::SymEntryType::_PROC)
                    {
                        // it is a procedure
                        if (res->second.defined == true)
                        {
                            // the procedure was already defined
                            // cannot redefine
                            analysis_error(node->line, std::string("Redefinition of procedure '") + res->first + "'.");
                        }
                        else
                        {
                            // this is a procedure definition and it hasn't been defined before
                            res->second.defined = true;
                        }
                        break;
                    }
                    else
                        analysis_error(node->line, std::string("The label '") + label->label_name + "' already exists; redefining");
                }
                else
                    symtable.add_entry(label->label_name, symtable::SymTableEntry(symtable::_LABEL));
                // since after the above conditional, the label will either be added to the symtable
                // or the assembling will halt due to error
                // so we can do the following
                if (label->label_name == "main")
                {
                    // we need to save the current pos
                    main_proc_index = inst_nodes.size();
                }
                break;
            }
            case nodes::NodeKind::_PROC_DECLR:
            {
                auto proc = (nodes::NodeProcDeclr *)node->ptr.get();
                // check if this label already exists
                // if so then it is not allowed to redefine the same label twice
                if (symtable.is_invalid(symtable.find_entry(proc->proc_name)))
                    analysis_error(node->line, std::string("The procedure '") + proc->proc_name + "' already exists; redefining");
                symtable.add_entry(proc->proc_name, symtable::SymTableEntry(symtable::_PROC));
                break;
            }
            }
            inst_nodes.push_back(std::move(node));
        }
    }
    // check if all the procedures have been correctly called
    check_proc_declr();
    // Now check if the instructions have valid symbols or not
    // the same loop
    for (auto &inst : inst_nodes)
    {
        // we only check for those instructions that may use variables and symbols
        switch (inst->kind)
        {
        case nodes::NodeKind::_INST_STORE:
        {
            auto node = (nodes::NodeStore *)inst->ptr.get();
            auto x = symtable.find_entry(node->var_name);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The operand '") + node->var_name + "' in the store instruction is not a valid identifier.");
            break;
        }
        case nodes::NodeKind::_INST_LOAD:
        {
            auto node = (nodes::NodeLoad *)inst->ptr.get();
            auto x = symtable.find_entry(node->var_name);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The operand '") + node->var_name + "' in the load instruction is not a valid identifier.");
            break;
        }
        case nodes::NodeKind::_INST_MOVF:
        case nodes::NodeKind::_INST_MOVLF:
        {
            // these instructions are different in the sense that they both accept variables of type df and dlf
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the movF instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_FLOAT && x->second.dtype != nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of FLOAT type to be used with 'movF'");
            }
            break;
        }
        case nodes::NodeKind::_INST_SIN:
        {
            auto node = (nodes::NodeOneImmOperand *)inst->ptr.get();
            auto x = symtable.find_entry(node->imm);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The operand '") + node->imm + "' in the sin instruction is not a valid identifier.");
            break;
        }
        case nodes::NodeKind::_INST_SOUT:
        {
            auto node = (nodes::NodeOneImmOperand *)inst->ptr.get();
            auto x = symtable.find_entry(node->imm);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The operand '") + node->imm + "' in the sout instruction is not a valid identifier.");
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_IMMQ:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_QWORD && x->second.dtype != nodes::DataType::_TYPE_RESQ)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of QWORD type to be used with 'movq'");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_IMM8:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_BYTE && x->second.dtype != nodes::DataType::_TYPE_STRING && x->second.dtype != nodes::DataType::_TYPE_RESB)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of BYTE type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_IMM16:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_WORD && x->second.dtype != nodes::DataType::_TYPE_RESW)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of WORD type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_IMM32:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_DWORD && x->second.dtype != nodes::DataType::_TYPE_RESD)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of DWORD type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOVSX_REG_IMM8:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_BYTE && x->second.dtype != nodes::DataType::_TYPE_RESB)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of BYTE type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOVSX_REG_IMM16:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_WORD && x->second.dtype != nodes::DataType::_TYPE_RESW)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of WORD type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOVSX_REG_IMM32:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
                if (x->second.dtype != nodes::DataType::_TYPE_DWORD && x->second.dtype != nodes::DataType::_TYPE_RESD)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of DWORD type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOV_REG_IMM:
        {
            auto node = (nodes::NodeInstMovRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                // the second operand is an identifier
                // we need to check if that is valid
                // we need codegen to be intelligent enough
                // to generate code based on the type of operands as well
                // as move doesn't have a variant for mov reg, memory
                // the codegen will have to either make it a load instruction
                // or generate error about the invalid operands
                // The best option here would be to add a move instruction variant for it
                // but we can't do that yet cause we need to keep the ISA's size in mind as well
                // so for now the best option would be to make the codgen intelligent
                if (!symtable.is_invalid(symtable.find_entry(node->value)))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the move instruction is not a valid identifier.");
            }
            break;
        }
        case nodes::NodeKind::_INST_ADD_IMM:
        {
            auto node = (nodes::NodeAddRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the add instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_SUB_IMM:
        {
            auto node = (nodes::NodeSubRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the sub instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MUL_IMM:
        {
            auto node = (nodes::NodeMulRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the mul instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_DIV_IMM:
        {
            auto node = (nodes::NodeDivRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the div instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_MOD_IMM:
        {
            auto node = (nodes::NodeModRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the mod instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_IADD_IMM:
        {
            auto node = (nodes::NodeAddRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the iadd instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_ISUB_IMM:
        {
            auto node = (nodes::NodeSubRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the isub instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_IMUL_IMM:
        {
            auto node = (nodes::NodeMulRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the imul instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_IDIV_IMM:
        {
            auto node = (nodes::NodeDivRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the idiv instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_IMOD_IMM:
        {
            auto node = (nodes::NodeModRegImm *)inst->ptr.get();
            if (node->is_iden)
            {
                auto x = symtable.find_entry(node->value);
                if (!symtable.is_invalid(x))
                    analysis_error(inst->line, std::string("The operand '") + node->value + "' in the imod instruction is not a valid identifier.");
                if (x->second.dtype == nodes::DataType::_TYPE_STRING || x->second.dtype == nodes::DataType::_TYPE_FLOAT || x->second.dtype == nodes::DataType::_TYPE_LFLOAT)
                    analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of NUM type as expected by the instruction.");
            }
            break;
        }
        case nodes::NodeKind::_INST_FADD_IMM:
        case nodes::NodeKind::_INST_LFADD_IMM:
        case nodes::NodeKind::_INST_FSUB_IMM:
        case nodes::NodeKind::_INST_LFSUB_IMM:
        case nodes::NodeKind::_INST_FMUL_IMM:
        case nodes::NodeKind::_INST_LFMUL_IMM:
        case nodes::NodeKind::_INST_FDIV_IMM:
        case nodes::NodeKind::_INST_LFDIV_IMM:
        {
            auto node = (nodes::NodeAddRegImm *)inst->ptr.get();
            auto x = symtable.find_entry(node->value);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The operand '") + node->value + "' in the floating-point instruction is not a valid identifier.");
            if (x->second.dtype != nodes::DataType::_TYPE_LFLOAT && x->second.dtype != nodes::DataType::_TYPE_FLOAT)
                analysis_error(inst->line, std::string("The variable '") + x->first + "' is not of FLOAT type as expected by the instruction.");
            break;
        }
        case nodes::NodeKind::_INST_JMP:
        case nodes::NodeKind::_INST_JNZ:
        case nodes::NodeKind::_INST_JZ:
        case nodes::NodeKind::_INST_JNE:
        case nodes::NodeKind::_INST_JE:
        case nodes::NodeKind::_INST_JNC:
        case nodes::NodeKind::_INST_JC:
        case nodes::NodeKind::_INST_JNO:
        case nodes::NodeKind::_INST_JO:
        case nodes::NodeKind::_INST_JNN:
        case nodes::NodeKind::_INST_JN:
        case nodes::NodeKind::_INST_JNG:
        case nodes::NodeKind::_INST_JG:
        case nodes::NodeKind::_INST_JNS:
        case nodes::NodeKind::_INST_JS:
        case nodes::NodeKind::_INST_JGE:
        case nodes::NodeKind::_INST_JSE:
        {
            auto node = (nodes::NodeJmp *)inst->ptr.get();
            auto x = symtable.find_entry(node->_jmp_label_);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The label ") + node->_jmp_label_ + " doesn't exist.");
            if (x->second.type != symtable::SymEntryType::_LABEL && x->second.type != symtable::SymEntryType::_PROC)
                analysis_error(inst->line, std::string("JMP to label ") + node->_jmp_label_ + " is not a label at all.");
            break;
        }
        case nodes::NodeKind::_INST_LOOP:
        {
            auto node = (nodes::NodeJmp *)inst->ptr.get();
            auto x = symtable.find_entry(node->_jmp_label_);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The label ") + node->_jmp_label_ + " doesn't exist.");
            if (x->second.type != symtable::SymEntryType::_LABEL && x->second.type != symtable::SymEntryType::_PROC)
                analysis_error(inst->line, std::string("LOOP to label ") + node->_jmp_label_ + " is not a label at all.");
            break;
        }
        case nodes::NodeKind::_INST_CALL:
        {
            auto node = (nodes::NodeCall *)inst->ptr.get();
            auto x = symtable.find_entry(node->_jmp_label_);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The procedure ") + node->_jmp_label_ + " doesn't exist.");
            if (x->second.type != symtable::SymEntryType::_PROC)
                analysis_error(inst->line, std::string("CALL to procedure ") + node->_jmp_label_ + " is not a procedure at all.");
            break;
        }
        case nodes::NodeKind::_INST_CMP_IMM:
        {
            auto node = (nodes::NodeCmpImm *)inst->ptr.get();
            if (!node->is_iden)
                break;
            auto x = symtable.find_entry(node->val);
            if (!symtable.is_invalid(x))
                analysis_error(inst->line, std::string("The variable ") + node->val + " doesn't exist.");
            break;
        }
        }
    }
}
/*
  By putting the labels and procedures in the symbol table, it facilitates error checking
  But at the end of the day, they are still just instructions
*/