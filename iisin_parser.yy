%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "2.5"
%defines
%define parser_class_name "iisin_parser"

%code requires {
#include <string>
#include "formula_ctlk.hh"
class iisin_driver;
}

// The parsing context
%parse-param { iisin_driver &driver }
%lex-param { iisin_driver &driver }

%locations
%initial-action
{
    // Initialise the initial location
    @$.begin.filename = @$.end.filename = &driver.file;
};

%debug
%error-verbose

// Symbols
%union
{
    int ival;
    std::string *sval;
    FormCTLK *fctlk;
};

%code {
#include "iisin_driver.hh"
}

%token SYSTEM CTLKFORM PROC TRANSITIONS LOCATIONS INITLOC AGENT
%token LCB RCB LRB RRB SEMICOL COLON COMMA RARR USCORE
%token AND OR IMPLIES NOT
%token EX EU EF EG AX AU AF AG NK
%token INF

%token        END      0 "end of file"
%token <sval> IDENTIFIER "identifier"
%token <ival> NUMBER     "number"

%left AND OR IMPLIES NOT
%left EX EU EF EG AX AU AF AG NK

%type <fctlk> ctlk_form
%type <ival> i_number;

//%printer    { yyoutput << *$$; } "identifier"
%destructor { delete $$; } "identifier"
//%printer    { yyoutput << $$; } <ival>

%%

%start sysElems;


sysElems:
    | sysElems sysElem
    ;

sysElem: 
    SYSTEM LCB procs RCB SEMICOL {
        if (driver.nosys)
        {
            FERROR("When loading compiled-in system, only properties can be specified");
        }
        driver.closeNet();
    }
    | CTLKFORM LCB ctlk_form RCB SEMICOL {
        driver.addFormCTLK($3);
        cout << (driver.getFormCTLK())->toStr() << endl;
    }
    ;

procs:
    | procs proc SEMICOL

proc:
    PROC IDENTIFIER LCB procElements RCB {
        driver.pushAutomaton(*$2);
    }
    ;

procElements:
    | procElements procElement SEMICOL
    ;

procElement:
    AGENT COLON NUMBER {
        driver.setCurAgent($3);
    }
    | INITLOC COLON IDENTIFIER {
        driver.setCurInit(*$3);
    }
    | TRANSITIONS LCB transitions RCB
    ;

transitions:
    | transitions transition SEMICOL
    ;

transition:
    | IDENTIFIER COLON IDENTIFIER RARR IDENTIFIER {
        driver.addCurTrans(*$1, *$3, *$5);
    }
    ;
ctlk_form: IDENTIFIER COLON IDENTIFIER {
        $$ = new FormCTLK(*$1, *$3);
    }
    | NOT ctlk_form {
        $$ = new FormCTLK(CTLK_NOT, $2);
    }
    | LRB ctlk_form RRB {
        $$ = $2;
    }
    | ctlk_form AND ctlk_form {
        $$ = new FormCTLK(CTLK_AND, $1, $3);
    }
    | ctlk_form OR ctlk_form {
        $$ = new FormCTLK(CTLK_OR, $1, $3);
    }
    | ctlk_form IMPLIES ctlk_form {
        $$ = new FormCTLK(CTLK_IMPL, $1, $3);
    }
    | EX ctlk_form {
        $$ = new FormCTLK(CTLK_EX, $2);
    }
    | EU LRB ctlk_form COMMA ctlk_form RRB {
        $$ = new FormCTLK(CTLK_EU, $3, $5);
    }
    | EU LRB i_number COMMA i_number COMMA ctlk_form COMMA ctlk_form RRB {
        $$ = new FormCTLK(CTLK_RT_EU, $3, $5, $7, $9);
    }
    | EF ctlk_form {
        $$ = new FormCTLK(CTLK_EF, $2);
    }
    | EF LRB i_number COMMA i_number COMMA ctlk_form RRB {
        $$ = new FormCTLK(CTLK_RT_EF, $3, $5, $7);
    }
    | EG ctlk_form {
        $$ = new FormCTLK(CTLK_EG, $2);
    }
    | EG LRB i_number COMMA i_number COMMA ctlk_form RRB {
        $$ = new FormCTLK(CTLK_RT_EG, $3, $5, $7);
    }
    | NK LRB NUMBER COMMA ctlk_form RRB {
        Agents ag;
        ag.push_back($3);
        $$ = new FormCTLK(CTLK_KD, ag, $5);
    }
    ;
i_number: NUMBER {
        $$ = $1;
    }
    | INF { 
        $$ = RT_INF;
    }
    ;

%%

void
yy::iisin_parser::error(const yy::iisin_parser::location_type &l, const std::string &m)
{
    driver.error(l, m);
}

