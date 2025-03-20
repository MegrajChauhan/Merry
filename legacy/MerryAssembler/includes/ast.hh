#if !defined(_MERRY_ASM_AST_)
#define _MERRY_ASM_AST_
#include "core.hh"
#include <variant>

namespace merry{
namespace front_end{
    enum struct AstInstType{
        NOP,
        HLT,
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        JMP,
        CALL,
        RET,
        OUTR,
        UOUTR,
    };
    class AstNodeInst{
        private:
            AstInstType _inst_type;
            std::vector<std::string> operands;
        public:
            AstNodeInst(AstInstType type = AstInstType::NOP) :_inst_type(type) {}
            void add_operand(std::string op) { operands.push_back(op); }
            AstInstType get_inst_type() { return _inst_type; }
            std::vector<std::string> get_operands() { return operands; }
            void clear_ops() { operands.clear(); }
            // Basiacly says if it's true it's OP_reg else it's OP_imm
            bool is_second_reg = false; // Should be filled by the Sema
    };
    class AstNodeLabel{
        private:
            std::string name;
            size_t addr; // Should be filled in by sema
        public:
            AstNodeLabel(std::string _name) :name(_name), addr(0) {}
            std::string get_name() { return name; }
            size_t get_addr() { return addr; }
            void set_addr(size_t _addr) { addr = _addr; }
    };
    class Ast{
        private:
            std::vector<std::variant<AstNodeInst, AstNodeLabel>> insts;
        public:
            Ast(){}
            void add_inst_or_label(std::variant<AstNodeInst, AstNodeLabel> inst) { insts.push_back(inst); }
            void print(){
                size_t i = 0;
                for(std::variant<AstNodeInst, AstNodeLabel> inst : insts){
                    std::cout << i << ": ";
                    if(inst.index() == 0){
                        std::cout << "Inst with type: " << (int)std::get<AstNodeInst>(inst).get_inst_type() << " Is reg inst = " << (std::get<AstNodeInst>(inst).is_second_reg ? "true" : "false") << std::endl;
                        std::cout << "  Operands:\n";
                        for(std::string op : std::get<AstNodeInst>(inst).get_operands()){
                            std::cout << "    " << op << std::endl;
                        }
                    }
                    ++i;
                }
            }
            void set_inst(size_t idx, std::variant<AstNodeInst, AstNodeLabel> node){
                insts[idx] = node;
            }
            std::vector<std::variant<AstNodeInst, AstNodeLabel>> get_insts(){
                return insts;
            }
    };
};
};

#endif // _MERRY_ASM_AST_
