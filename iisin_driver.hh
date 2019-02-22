#ifndef IISIN_DRIVER_HH
#define IISIN_DRIVER_HH
#include <string>
#include <map>
#include "iisin_parser.hh"
#include "automata.hh"
#include "formula_ctlk.hh"

// Tell Flex the lexer's prototype ...
#define YY_DECL                                  \
  yy::iisin_parser::token_type                    \
  yylex(yy::iisin_parser::semantic_type* yylval,  \
         yy::iisin_parser::location_type* yylloc, \
         iisin_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

// Conducting the whole scanning an parsing of RS
class iisin_driver
{
public:
    iisin_driver(SymAutomataNet *net);
    virtual ~iisin_driver();

    //std::map<std::string, int> variables;
    SymAutomataNet *net;
    FormCTLK *ctlkform;

    Automaton *cur_aut;

    // Handling the scanner
    void scan_begin();
    void scan_end();
    bool trace_scanning;

    int parse(const std::string &f);
    std::string file;
    bool trace_parsing;

    // IIS-specific
    
    bool nosys;

    void pushAutomaton(const std::string &name);
    void setCurInit(const std::string &name) {
        cur_aut->setInitLocation(name);
    }
    void addCurTrans(const std::string &src, const std::string &dst, const std::string &name) {
        cur_aut->addTransition(src, dst, name);
    }
    void setCurAgent(int id) { cur_aut->setAgent(id); }
    void closeNet(void) { net->closeNet(); }

    void addFormCTLK(FormCTLK *f) { ctlkform = f; }
    FormCTLK *getFormCTLK(void);

    // Error handling.
    void error(const yy::location &l, const std::string &m);
    void error(const std::string &m);
};

#endif


