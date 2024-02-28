#include "../includes/sema.hpp"

masm::sema::Sema::Sema(parser::Parser& parser)
{
    parser.parse();
    // nodes = parser.get_nodes();
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
    for (auto& node : nodes)
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
                {
                    analysis_error(node->line, std::string("The variable '") + var->byte_name + "' already exists; redefining");
                }
                symtable.add_entry(var->byte_name, symtable::SymTableEntry(symtable::_VAR, var->byte_val, nodes::_TYPE_NUM));
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
                if (symtable.is_invalid(symtable.find_entry(label->label_name)))
                {
                    analysis_error(node->line, std::string("The label ") + label->label_name + " already exists; redefining");
                }
                symtable.add_entry(label->label_name, symtable::SymTableEntry(symtable::_LABEL));
                break;
            }
            case nodes::NodeKind::_PROC_DECLR:
            {
                auto proc = (nodes::NodeProcDeclr *)node->ptr.get();
                // check if this label already exists
                // if so then it is not allowed to redefine the same label twice
                if (symtable.is_invalid(symtable.find_entry(proc->proc_name)))
                {
                    analysis_error(node->line, std::string("The procedure ") + proc->proc_name + " already exists; redefining");
                }
                symtable.add_entry(proc->proc_name, symtable::SymTableEntry(symtable::_PROC));
                break;
            }
            }
            inst_nodes.push_back(std::make_unique<nodes::Node>(*(node.get())));
        }
    }
}

/*
  By putting the labels and procedures in the symbol table, it facilitates error checking
  But at the end of the day, they are still just instructions
*/