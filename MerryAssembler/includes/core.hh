#if !defined(_MERRY_ASM_CORE_)
#define _MERRY_ASM_CORE_
#include <cstddef>
#include <string>
#include <vector>
#include <iostream>

namespace merry{
namespace core{
    class Loc{
        private:
            size_t col, row;
            std::string file_name;
        public:
            Loc(size_t col, size_t row, std::string file_name){
                this->col = col;
                this->row = row;
                this->file_name = file_name;
            }
            void print(){
                std::cout << file_name << ":" << col << ":" << row;
            }
            void update(int c);
    };
};
};

#endif // _MERRY_ASM_CORE_
