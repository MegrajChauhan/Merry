#ifndef _COMPUNIT_
#define _COMPUNIT_

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <memory>
#include "error.hpp"

namespace masm
{
    enum DataType
    {
        _TYPE_BYTE,
    };

    struct Pos
    {
        size_t line;
        size_t col_st;
        size_t col_end;
        std::shared_ptr<std::string> _file = nullptr;
    };

    struct Variable
    {
        std::string value;
        DataType type;
        Pos pos;
    };

    struct Definitions
    {
        std::string value;
        Pos pos;
    };

    class CompUnit
    {
        std::unordered_map<std::string, Definitions> _def_table;
        std::unordered_map<std::string, Variable> _var_table;
        std::filesystem::path path;
        size_t line = 0, col = 0;
        size_t offset = 0;

    public:
        CompUnit(std::string _path);

        bool find_def(std::string);
        bool find_var(std::string);


    };
};

#endif