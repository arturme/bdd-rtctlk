%{ /* -*- C++ -*- */
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <string>
#include "iisin_driver.hh"
#include "iisin_parser.hh"

/* Work around an incompatibility in flex (at least versions
   2.5.31 through 2.5.33): it generates code that does
   not conform to C89.  See Debian bug 333231
   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
#undef yywrap
#define yywrap() 1

/* By default yylex returns int, we use token_type.
   Unfortunately yyterminate by default returns 0, which is
   not of token_type.  */
#define yyterminate() return token::END
%}

%option noyywrap nounput batch debug

id [a-zA-Z][a-zA-Z_0-9]*
int [0-9]+
blank [ \t]

%{
#define YY_USER_ACTION  yylloc->columns(yyleng);
%}
%%
%{
  yylloc->step();
%}
{blank}+   yylloc->step();
[\n]+      yylloc->lines(yyleng); yylloc->step();
%{
  typedef yy::iisin_parser::token token;
%}

"system"                return token::SYSTEM;
"proc"                  return token::PROC;
"agent"                 return token::AGENT;
"trans"                 return token::TRANSITIONS;
"locations"             return token::LOCATIONS;
"init"                  return token::INITLOC;
"ctlk-prop"             return token::CTLKFORM;
"{"                     return token::LCB;
"}"                     return token::RCB;
"("                     return token::LRB;
")"                     return token::RRB;
";"                     return token::SEMICOL;
":"                     return token::COLON;
","                     return token::COMMA;
"->"                    return token::RARR;
"_"                     return token::USCORE;
"AND"                   return token::AND;
"OR"                    return token::OR;
"IMPLIES"               return token::IMPLIES;
"~"                     return token::NOT;
"EX"                    return token::EX;
"EU"                    return token::EU;
"EF"                    return token::EF;
"EG"                    return token::EG;
"AX"                    return token::AX;
"AU"                    return token::AU;
"AF"                    return token::AF;
"AG"                    return token::AG;
"NK"                    return token::NK;
"inf"                   return token::INF;
"#".*                   ;

{int}    {
           errno = 0;
           long n = strtol(yytext, NULL, 10);
           if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
             driver.error(*yylloc, "integer is out of range");
           yylval->ival = n;
           return token::NUMBER;
         }

{id}     {
           yylval->sval = new std::string(yytext);
           return token::IDENTIFIER;
         }

.        driver.error(*yylloc, "invalid character");
%%

void
iisin_driver::scan_begin()
{
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      error("cannot open " + file + ": " + strerror(errno));
      exit(EXIT_FAILURE);
    }
}

void
iisin_driver::scan_end()
{
  fclose(yyin);
}

