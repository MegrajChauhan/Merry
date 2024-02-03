#include "../includes/sema.hh"
#include <algorithm>

std::vector<std::string> valid_registers = {
    "Ma",
    "Mb",
    "Mc",
    "Md",
    "Me",
    "Mf",
    "M1",
    "M2",
    "M3",
    "M4",
    "M5",
    "Mm1",
    "Mm2",
    "Mm3",
    "Mm4",
    "Mm5"
};

std::vector<merry::front_end::AstInstType> first_op_has_register = {
    merry::front_end::AstInstType::ADD,
};

merry::front_end::AstNodeInst merry::front_end::Sema::resolve_inst_imm_or_reg(AstNodeInst inst){
    AstNodeInst copy_inst = inst;
    if(copy_inst.get_operands().size() == 0){
        return inst;
    }
    if(std::find(first_op_has_register.begin(), first_op_has_register.end(), copy_inst.get_inst_type()) != first_op_has_register.end()){
        if(std::find(valid_registers.begin(), valid_registers.end(), copy_inst.get_operands().at(0)) == valid_registers.end()){
            std::cout << "Encountered an invalid register name `" << copy_inst.get_operands().at(0) << "`\n";
            return inst;
        }
    }
    if(std::find(valid_registers.begin(), valid_registers.end(), copy_inst.get_operands().at(1)) != valid_registers.end()){
        copy_inst.is_second_reg = true;
    }
    if(copy_inst.is_second_reg){
        if(std::find(valid_registers.begin(), valid_registers.end(), copy_inst.get_operands().at(1)) == valid_registers.end()){
            std::cout << "Encountered an invalid register name `" << copy_inst.get_operands().at(1) << "`\n";
            return inst;
        }
    }
    return copy_inst;
}
merry::front_end::AstNodeLabel merry::front_end::Sema::resolve_label_addr(AstNodeLabel label){
    label.set_addr(copy_ast.get_insts().size());
    return label;
}