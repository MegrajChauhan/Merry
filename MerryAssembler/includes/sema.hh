#if !defined(_MERRY_ASM_SEMA_)
#define _MERRY_ASM_SEMA_
#include "ast.hh"

namespace merry{
namespace front_end{
    class Sema{
        private:
            Ast in_ast;
            Ast copy_ast;
            bool find_dub_label(std::string label1_name){
                for(auto inst : copy_ast.get_insts()){
                    if(inst.index() == 1){
                        AstNodeLabel label = std::get<AstNodeLabel>(inst);
                        if(label.get_name() == label1_name) return true;
                    }
                }
                return false;
            }
            size_t insts_size; // To correctly remove the labels the jmp addresses should be the number of insts currently parsed this way no labels have to be in the IR
            std::vector<std::pair<size_t, std::string>> unresolved_jmps;
            size_t find_label_addr(std::string name){
                for(auto inst : copy_ast.get_insts()){
                    // Make sure it's a label inst
                    if(inst.index() == 1){
                        AstNodeLabel label = std::get<AstNodeLabel>(inst);
                        if(label.get_name() == name){
                            return label.get_addr();
                        }
                    }
                }
                std::cout << "No label found with name: " << name << "!!!\n";
                exit(1);
            }
        public:
            Sema(Ast ast) :in_ast(ast) { insts_size = 0; }
            AstNodeInst resolve_inst_imm_or_reg(AstNodeInst inst);
            AstNodeLabel resolve_label_addr(AstNodeLabel label);
            Ast resolve_ast(){
                std::vector<std::variant<AstNodeInst, AstNodeLabel>> insts = in_ast.get_insts();
                for(size_t i = 0; i < insts.size(); ++i){
                    std::variant<AstNodeInst, AstNodeLabel> inst = insts[i];
                    if(inst.index() == 0){
                        copy_ast.add_inst_or_label(resolve_inst_imm_or_reg(std::get<AstNodeInst>(inst)));
                    } else if(inst.index() == 1){
                        copy_ast.add_inst_or_label(resolve_label_addr(std::get<AstNodeLabel>(inst)));
                    }
                }
                for(std::pair<size_t, std::string> jmp : unresolved_jmps){
                    size_t label_addr = find_label_addr(jmp.second);
                    AstNodeInst inst = std::get<AstNodeInst>(copy_ast.get_insts().at(jmp.first));
                    inst.clear_ops();
                    inst.add_operand(std::to_string(label_addr));
                    copy_ast.set_inst(jmp.first, inst);
                }
                Ast copy2_ast;
                for(std::variant<AstNodeInst, AstNodeLabel> inst : copy_ast.get_insts()){
                    if(inst.index() == 0){
                        copy2_ast.add_inst_or_label(inst);
                    }
                }
                return copy2_ast;
            }
    };
};
};

#endif // _MERRY_ASM_SEMA_
