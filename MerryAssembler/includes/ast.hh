#if !defined(_MERRY_ASM_AST_)
#define _MERRY_ASM_AST_
#include "core.hh"
#include <variant>

namespace merry{
namespace front_end{
    enum struct AstInstType{
        NOP,
        HLT,
    };
    class AstNodeInst{
        private:
            // TODO: Intoduce operands
            AstInstType _inst_type;
        public:
            AstNodeInst(AstInstType type = AstInstType::NOP) :_inst_type(type) {}
            AstInstType get_inst_type() { return _inst_type; }
    };
    class AstNodeLabel{
        private:
            // TODO: Intoduce operands
            std::string name;
        public:
            AstNodeLabel(std::string _name) :name(_name) {}
            std::string get_name() { return name; }
    };
    class Ast{
        private:
            std::vector<std::variant<AstNodeInst, AstNodeLabel>> insts;
        public:
            Ast(){}
            void add_inst_or_label(std::variant<AstNodeInst, AstNodeLabel> inst) { insts.push_back(inst); }
            void print(){
                for(std::variant<AstNodeInst, AstNodeLabel> inst : insts){
                    if(inst.index() == 0){
                        std::cout << "Inst with type: " << (int)std::get<AstNodeInst>(inst).get_inst_type() << std::endl;
                    } else if(inst.index() == 1){
                        std::cout << "Label with name: " << std::get<AstNodeLabel>(inst).get_name() << std::endl;
                    }
                }
            }
    };
};
};

#endif // _MERRY_ASM_AST_
