#include "../includes/ir.hh"

std::vector<std::pair<std::string, uint8_t>> registers = {
    {"Ma", 0x00},
    {"Mb", 0x11},
    {"Mc", 0x22},
    {"Md", 0x33},
    {"Me", 0x44},
    {"Mf", 0x55},
    {"M1", 0x66},
    {"M2", 0x77},
    {"M3", 0x88},
    {"M4", 0x99},
    {"M5", 0xAA},
    {"Mm1", 0xBB},
    {"Mm2", 0xCC},
    {"Mm3", 0xDD},
    {"Mm4", 0xEE},
    {"Mm5", 0xFF},
};

inline static uint8_t register_to_byte(std::string regis){
    for(std::pair<std::string, uint8_t> reg : registers){
        if(reg.first == regis){
            return reg.second;
        }
    }
    return 0;
}

void merry::back_end::IrGen::convert_astnode_to_inst(){
    front_end::AstInstType ast_inst_type = current_node.get_inst_type();
    IrInst inst;
    switch(ast_inst_type){
        case front_end::AstInstType::NOP: {
            inst = IrInst(IrInstType::NOP);
        } break;
        case front_end::AstInstType::HLT: {
            inst = IrInst(IrInstType::HLT);
        } break;
        case front_end::AstInstType::RET: {
            inst = IrInst(IrInstType::RET);
        } break;

        // Mathn't
        case front_end::AstInstType::ADD: {
            // Check if it's a register instruction
            if(current_node.is_second_reg){
                inst = IrInst(IrInstType::ADD_REG);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits
                uint8_t top_4_bits_to_set = register_to_byte(current_node.get_operands().at(1));   // Example value for top 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                // Setting top 4 bits
                registers = (registers & 0x0F) | (top_4_bits_to_set & 0xF0);
                inst.add_operand(registers);
            } else{
                inst = IrInst(IrInstType::ADD_IMM);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                inst.add_operand(registers);
                const char *str = current_node.get_operands().at(1).c_str();
                char* endptr;
                uint32_t value = strtol(str, &endptr, 10);
                if (endptr == str) {
                  std::cout << "No digits were found\n";
                  exit(1);
                } else if (*endptr != '\0') {
                  std::cout << "Invalid input: " << str << std::endl;
                  exit(1);
                }
                uint8_t lower4_bytes =        ((value >> 8*0) & 0x000000FF); // lower 4 bits
                uint8_t lower_middle4_bytes = ((value >> 8*1) & 0x000000FF); // Next 4 bits
                uint8_t top_middle4_bytes =   ((value >> 8*2) & 0x000000FF); // Next 4 bits
                uint8_t top4_bytes =          ((value >> 8*3) & 0x000000FF); // top 4 bits
                inst.add_operand(lower4_bytes);
                inst.add_operand(lower_middle4_bytes);
                inst.add_operand(top_middle4_bytes);
                inst.add_operand(top4_bytes);
            }
        } break;
        case front_end::AstInstType::SUB: {
            // Check if it's a register instruction
            if(current_node.is_second_reg){
                inst = IrInst(IrInstType::SUB_REG);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits
                uint8_t top_4_bits_to_set = register_to_byte(current_node.get_operands().at(1));   // Example value for top 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                // Setting top 4 bits
                registers = (registers & 0x0F) | (top_4_bits_to_set & 0xF0);
                inst.add_operand(registers);
            } else{
                inst = IrInst(IrInstType::SUB_IMM);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                inst.add_operand(registers);
                const char *str = current_node.get_operands().at(1).c_str();
                char* endptr;
                uint32_t value = strtol(str, &endptr, 10);
                if (endptr == str) {
                  std::cout << "No digits were found\n";
                  exit(1);
                } else if (*endptr != '\0') {
                  std::cout << "Invalid input: " << str << std::endl;
                  exit(1);
                }
                uint8_t lower4_bytes =        ((value >> 8*0) & 0x000000FF); // lower 4 bits
                uint8_t lower_middle4_bytes = ((value >> 8*1) & 0x000000FF); // Next 4 bits
                uint8_t top_middle4_bytes =   ((value >> 8*2) & 0x000000FF); // Next 4 bits
                uint8_t top4_bytes =          ((value >> 8*3) & 0x000000FF); // top 4 bits
                inst.add_operand(lower4_bytes);
                inst.add_operand(lower_middle4_bytes);
                inst.add_operand(top_middle4_bytes);
                inst.add_operand(top4_bytes);
            }
        } break;
        case front_end::AstInstType::MOD: {
            // Check if it's a register instruction
            if(current_node.is_second_reg){
                inst = IrInst(IrInstType::MOD_REG);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits
                uint8_t top_4_bits_to_set = register_to_byte(current_node.get_operands().at(1));   // Example value for top 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                // Setting top 4 bits
                registers = (registers & 0x0F) | (top_4_bits_to_set & 0xF0);
                inst.add_operand(registers);
            } else{
                inst = IrInst(IrInstType::MOD_IMM);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                inst.add_operand(registers);
                const char *str = current_node.get_operands().at(1).c_str();
                char* endptr;
                uint32_t value = strtol(str, &endptr, 10);
                if (endptr == str) {
                  std::cout << "No digits were found\n";
                  exit(1);
                } else if (*endptr != '\0') {
                  std::cout << "Invalid input: " << str << std::endl;
                  exit(1);
                }
                uint8_t lower4_bytes =        ((value >> 8*0) & 0x000000FF); // lower 4 bits
                uint8_t lower_middle4_bytes = ((value >> 8*1) & 0x000000FF); // Next 4 bits
                uint8_t top_middle4_bytes =   ((value >> 8*2) & 0x000000FF); // Next 4 bits
                uint8_t top4_bytes =          ((value >> 8*3) & 0x000000FF); // top 4 bits
                inst.add_operand(lower4_bytes);
                inst.add_operand(lower_middle4_bytes);
                inst.add_operand(top_middle4_bytes);
                inst.add_operand(top4_bytes);
            }
        } break;
        case front_end::AstInstType::DIV: {
            // Check if it's a register instruction
            if(current_node.is_second_reg){
                inst = IrInst(IrInstType::DIV_REG);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits
                uint8_t top_4_bits_to_set = register_to_byte(current_node.get_operands().at(1));   // Example value for top 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                // Setting top 4 bits
                registers = (registers & 0x0F) | (top_4_bits_to_set & 0xF0);
                inst.add_operand(registers);
            } else{
                inst = IrInst(IrInstType::DIV_IMM);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                inst.add_operand(registers);
                const char *str = current_node.get_operands().at(1).c_str();
                char* endptr;
                uint32_t value = strtol(str, &endptr, 10);
                if (endptr == str) {
                  std::cout << "No digits were found\n";
                  exit(1);
                } else if (*endptr != '\0') {
                  std::cout << "Invalid input: " << str << std::endl;
                  exit(1);
                }
                uint8_t lower4_bytes =        ((value >> 8*0) & 0x000000FF); // lower 4 bits
                uint8_t lower_middle4_bytes = ((value >> 8*1) & 0x000000FF); // Next 4 bits
                uint8_t top_middle4_bytes =   ((value >> 8*2) & 0x000000FF); // Next 4 bits
                uint8_t top4_bytes =          ((value >> 8*3) & 0x000000FF); // top 4 bits
                inst.add_operand(lower4_bytes);
                inst.add_operand(lower_middle4_bytes);
                inst.add_operand(top_middle4_bytes);
                inst.add_operand(top4_bytes);
            }
        } break;
        case front_end::AstInstType::MUL: {
            // Check if it's a register instruction
            if(current_node.is_second_reg){
                inst = IrInst(IrInstType::MUL_REG);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits
                uint8_t top_4_bits_to_set = register_to_byte(current_node.get_operands().at(1));   // Example value for top 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                // Setting top 4 bits
                registers = (registers & 0x0F) | (top_4_bits_to_set & 0xF0);
                inst.add_operand(registers);
            } else{
                inst = IrInst(IrInstType::MUL_IMM);
                uint8_t registers = 0; // Example registers
                uint8_t lower_4_bits_to_set = register_to_byte(current_node.get_operands().at(0)); // Example value for lower 4 bits

                // Setting lower 4 bits
                registers = (registers & 0xF0) | (lower_4_bits_to_set & 0x0F);

                inst.add_operand(registers);
                const char *str = current_node.get_operands().at(1).c_str();
                char* endptr;
                uint32_t value = strtol(str, &endptr, 10);
                if (endptr == str) {
                  std::cout << "No digits were found\n";
                  exit(1);
                } else if (*endptr != '\0') {
                  std::cout << "Invalid input: " << str << std::endl;
                  exit(1);
                }
                uint8_t lower4_bytes =        ((value >> 8*0) & 0x000000FF); // lower 4 bits
                uint8_t lower_middle4_bytes = ((value >> 8*1) & 0x000000FF); // Next 4 bits
                uint8_t top_middle4_bytes =   ((value >> 8*2) & 0x000000FF); // Next 4 bits
                uint8_t top4_bytes =          ((value >> 8*3) & 0x000000FF); // top 4 bits
                inst.add_operand(lower4_bytes);
                inst.add_operand(lower_middle4_bytes);
                inst.add_operand(top_middle4_bytes);
                inst.add_operand(top4_bytes);
            }
        } break;
        // End Mathn't
        case front_end::AstInstType::CALL: {
            inst = IrInst(IrInstType::CALL);
            const char *str = current_node.get_operands().at(0).c_str();
            char* endptr;
            uint64_t value = strtoll(str, &endptr, 10);
            if (endptr == str) {
              std::cout << "No digits were found\n";
              exit(1);
            } else if (*endptr != '\0') {
              std::cout << "Invalid input: " << str << std::endl;
              exit(1);
            }
            uint8_t upper_a = uint8_t((value >> 56) & 0xff);
            uint8_t lower_a = uint8_t((value >> 48) & 0xff);
            uint8_t upper_b = uint8_t((value >> 40) & 0xff);
            uint8_t lower_b = uint8_t((value >> 32) & 0xff);
            uint8_t upper_c = uint8_t((value >> 24) & 0xff);
            uint8_t lower_c = uint8_t((value >> 16) & 0xff);
            uint8_t upper_d = uint8_t((value >> 8) & 0xff);
            uint8_t lower_d = uint8_t((value >> 0) & 0xff);
            inst.add_operand(lower_d);
            inst.add_operand(upper_d);
            inst.add_operand(lower_c);
            inst.add_operand(upper_c);
            inst.add_operand(lower_b);
            inst.add_operand(upper_b);
            inst.add_operand(lower_a);
            inst.add_operand(upper_a);
        } break;
        case front_end::AstInstType::JMP: {
            inst = IrInst(IrInstType::JMP_ADDR);
            const char *str = current_node.get_operands().at(0).c_str();
            char* endptr;
            uint64_t value = strtoll(str, &endptr, 10);
            if (endptr == str) {
              std::cout << "No digits were found\n";
              exit(1);
            } else if (*endptr != '\0') {
              std::cout << "Invalid input: " << str << std::endl;
              exit(1);
            }
            uint8_t upper_a = uint8_t((value >> 56) & 0xff);
            uint8_t lower_a = uint8_t((value >> 48) & 0xff);
            uint8_t upper_b = uint8_t((value >> 40) & 0xff);
            uint8_t lower_b = uint8_t((value >> 32) & 0xff);
            uint8_t upper_c = uint8_t((value >> 24) & 0xff);
            uint8_t lower_c = uint8_t((value >> 16) & 0xff);
            uint8_t upper_d = uint8_t((value >> 8) & 0xff);
            uint8_t lower_d = uint8_t((value >> 0) & 0xff);
            inst.add_operand(lower_d);
            inst.add_operand(upper_d);
            inst.add_operand(lower_c);
            inst.add_operand(upper_c);
            inst.add_operand(lower_b);
            inst.add_operand(upper_b);
            inst.add_operand(lower_a);
            inst.add_operand(upper_a);
        } break;
    }
    add_inst(inst);

}