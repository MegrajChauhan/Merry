#include "expr.hpp"

/**
 * not
 * multiply and division,
 * addition and subtraction
 * shifts
 * >, <, <=, >=
 * == !=
 * ~
 * &
 * ^
 * |
 * and
 * or
 *
 */
static int get_precedence(masm::TokenType t)
{
    switch (t)
    {
    case masm::TokenType::OPER_OR:
        return 0;
    case masm::TokenType::OPER_AND:
        return 1;
    case masm::TokenType::OPER_LOR:
        return 2;
    case masm::TokenType::OPER_XOR:
        return 3;
    case masm::TokenType::OPER_LAND:
        return 4;
    case masm::TokenType::OPER_LNOT:
        return 5;
    case masm::TokenType::OPER_EQ:
    case masm::TokenType::OPER_NEQ:
        return 6;
    case masm::TokenType::OPER_GT:
    case masm::TokenType::OPER_LT:
    case masm::TokenType::OPER_GE:
    case masm::TokenType::OPER_LE:
        return 7;
    case masm::TokenType::OPER_LS:
    case masm::TokenType::OPER_RS:
        return 8;
    case masm::TokenType::OPER_PLUS:
    case masm::TokenType::OPER_MINUS:
        return 9;
    case masm::TokenType::OPER_MUL:
    case masm::TokenType::OPER_DIV:
        return 10;
    case masm::TokenType::OPER_NOT:
        return 11;
    }
}

/*
  To keep things simple and easy, we will skip precedence completely.
  We may take the precedence to be left to right for all operators
*/

void masm::Expr::add_expr(std::vector<Token> _e)
{
    expr = _e;
}

void masm::Expr::add_table(SymbolTable *t)
{
    table = t;
}

std::optional<std::string> masm::Expr::evaluate()
{
    std::string res;
    for (auto t : expr)
    {
        switch (t.type)
        {
        case TokenType::OPER_OPEN_PAREN:
            opers.push(t.type);
            break;
        case TokenType::NUM_INT:
        case TokenType::NUM_FLOAT:
            operads.push(std::stod(t.val));
            break;
        case TokenType::IDENTIFIER:
        {
            // get it's value
            auto res = table->_var_list.find(t.val);
            if (!(res == table->_var_list.end()))
            {
                auto v = table->variables[res->second];
                switch (v.type)
                {
                case DataType::RESB:
                case DataType::RESW:
                case DataType::RESD:
                case DataType::RESQ:
                case DataType::STRING:
                    note("Unexpected token in expression.");
                    return {};
                }
                operads.push(std::stod(v.value));
            }
            else
            {
                auto res1 = table->_const_list.find(t.val);
                if (res1 == table->_const_list.end())
                {
                    note("Expected a variable or a constant that exists.");
                    return {};
                }
                operads.push(std::stod(res1->second.value));
            }
            break;
        }
        case TokenType::OPER_NOT:
        {
            opers.push(t.type);
            break;
        }
        case TokenType::OPER_AND:
        case TokenType::OPER_OR:
        case TokenType::OPER_XOR:
        case TokenType::OPER_LS:
        case TokenType::OPER_RS:
        case TokenType::OPER_LAND:
        case TokenType::OPER_LOR:
        case TokenType::OPER_LNOT:
        case TokenType::OPER_LT:
        case TokenType::OPER_GT:
        case TokenType::OPER_LE:
        case TokenType::OPER_GE:
        case TokenType::OPER_EQ:
        case TokenType::OPER_NEQ:
        case TokenType::OPER_PLUS:
        case TokenType::OPER_MINUS:
        case TokenType::OPER_MUL:
        case TokenType::OPER_DIV:
        {
            if (!opers.empty() && (get_precedence(opers.top()) >= get_precedence(t.type)))
                if (!perform())
                    return {};
            opers.push(t.type);
            break;
        }
        case TokenType::OPER_CLOSE_PAREN:
        {
            while (!opers.empty() && opers.top() != TokenType::OPER_OPEN_PAREN)
            {
                if (!perform())
                    return {};
            }
            if (!opers.empty() && opers.top() != TokenType::OPER_OPEN_PAREN)
            {
                note("Invalid expression: Stray PARENTHESIS.");
                return {};
            }
            break;
        }
        default:
            note("Unknown token in expression.");
            return {};
        }
    }
    while (!opers.empty())
    {
        if (!perform())
            return {};
    }
    if (!operads.empty())
    {
        note("Invalid expression.");
        return {};
    }
    return std::make_optional<std::string>(std::to_string(operads.top()));
}

bool masm::Expr::perform()
{
    switch (opers.top())
    {
    case TokenType::OPER_AND:
    {
        if (operads.size() < 2)
        {
            note("AND operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((o1 > 0.0 && 02 > 0.0) ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_OR:
    {
        if (operads.size() < 2)
        {
            note("OR operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((o1 > 0.0 || 02 > 0.0) ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_XOR:
    {
        if (operads.size() < 2)
        {
            note("XOR operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((double)((size_t)o1 ^ (size_t)o2));
        break;
    }
    case TokenType::OPER_NOT:
    {
        if (operads.size() < 1)
        {
            note("NOT operator requires one operand.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        operads.push((double)(o1 > 0 ? 0 : 1));
        break;
    }
    case TokenType::OPER_LS:
    {
        if (operads.size() < 2)
        {
            note("LEFT SHIFT operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((double)((size_t)o1 >> (size_t)o2));
        break;
    }
    case TokenType::OPER_RS:
    {
        if (operads.size() < 2)
        {
            note("RIGHT SHIFT operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((double)((size_t)o1 << (size_t)o2));
        break;
    }
    case TokenType::OPER_LAND:
    {
        if (operads.size() < 2)
        {
            note("LOGICAL AND operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((double)((size_t)o1 & (size_t)o2));
        break;
    }
    case TokenType::OPER_LOR:
    {
        if (operads.size() < 2)
        {
            note("LOGICAL OR operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push((double)((size_t)o1 | (size_t)o2));
        break;
    }
    case TokenType::OPER_LNOT:
    {
        if (operads.size() < 1)
        {
            note("LOGICAL NOT operator requires one operand.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        operads.push((double)(~(size_t)o1));
        break;
    }
    case TokenType::OPER_LT:
    {
        if (operads.size() < 2)
        {
            note("LESS THAN operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 < o2 ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_GT:
    {
        if (operads.size() < 2)
        {
            note("GREATER THAN operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 > o2 ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_LE:
    {
        if (operads.size() < 2)
        {
            note("LESS THAN OR EQUAL operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 <= o2 ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_GE:
    {
        if (operads.size() < 2)
        {
            note("GREATER THAN OR EQUAL operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 >= o2 ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_EQ:
    {
        if (operads.size() < 2)
        {
            note("EQUALS operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 == o2 ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_NEQ:
    {
        if (operads.size() < 2)
        {
            note("NOT EQUALS operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 != o2 ? 1.0 : 0.0);
        break;
    }
    case TokenType::OPER_PLUS:
    {
        if (operads.size() < 2)
        {
            note("PLUS operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 + o2);
        break;
    }
    case TokenType::OPER_MINUS:
    {
        if (operads.size() < 2)
        {
            note("MINUS operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 - o2);
        break;
    }
    case TokenType::OPER_MUL:
    {
        if (operads.size() < 2)
        {
            note("MULTIPLY operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        operads.push(o1 * o2);
        break;
    }
    case TokenType::OPER_DIV:
    {
        if (operads.size() < 2)
        {
            note("DIVIDE operator requires two operands.");
            return false;
        }
        double o1 = operads.top();
        operads.pop();
        double o2 = operads.top();
        operads.pop();
        if (o2 == 0)
        {
            note("DIV BY ZERO.");
            return false;
        }
        operads.push(o1 / o2);
        break;
    }
    }
    opers.pop();
    return true;
}