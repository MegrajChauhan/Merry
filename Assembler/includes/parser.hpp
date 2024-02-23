#ifndef _PARSER_
#define _PARSER_

#include "lexer.hpp"
#include "nodes.hpp"

namespace masm
{
    namespace parser
    {
        enum CurrentSection
        {
            // anything must belong to a section else an error will be thrown
            _SECTION_NONE,
            _SECTION_DATA,
            _SECTION_TEXT,
        };

        class Parser
        {
            lexer::Lexer lexer;
            CurrentSection section = _SECTION_NONE;
            std::vector<nodes::Node> nodes; // the parsed nodes

        public:
            Parser() = default;

            Parser(lexer::Lexer);

            // setup by file path
            void setup_lexer(std::string);
        };
    };
};

#endif