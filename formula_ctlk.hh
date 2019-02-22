

#ifndef FORMULA_CTLK_HH_AECPEQN
#define FORMULA_CTLK_HH_AECPEQN

#include <string>
#include <sstream>
#include <climits>
#include <cassert>
#include "cudd/include/cuddObj.hh"
#include "mctools.hh"
#include "automata.hh"

// CTLK
#define CTLK_PV 0 // propositional variable
#define CTLK_AND 1
#define CTLK_OR 2
#define CTLK_NOT 3
#define CTLK_IMPL 4
#define CTLK_EG 11
#define CTLK_EU 12
#define CTLK_EX 13
#define CTLK_EF 14
#define CTLK_KD 20
#define CTLK_OD 30
#define CTLK_TF 50 // true/false
#define CTLK_RT_EG 60
#define CTLK_RT_EU 61
#define CTLK_RT_EF 62

#define RT_INF INT_MAX

typedef unsigned char Oper;
typedef std::vector<unsigned int> Agents;
class FormCTLK
{
    Oper oper;
    FormCTLK *arg[2];
    string aut;
    string loc;
    bool tf;
    BDD *bdd;
    int left;
    int right;
    Agents agents;
public:
    FormCTLK(string aut, string loc);
    //FormCTLK(string varName, BDD varBDD);
    FormCTLK(bool val);
    FormCTLK(Oper op, FormCTLK *form1, FormCTLK *form2);
    FormCTLK(Oper op, FormCTLK *form1);
    FormCTLK(Oper op, Agents &agents, FormCTLK *form1);
    FormCTLK(Oper op, int a, int b, FormCTLK *form1);
    FormCTLK(Oper op, int a, int b, FormCTLK *form1, FormCTLK *form2);
    string toStr(void);
    BDD satStates(const BDD &reach);
    void cleanup(void);
    ~FormCTLK();
    unsigned int getAgent(void)
    {
        assert(oper == CTLK_KD);
        return agents[0];
    }
    Oper getOper(void) const
    {
        return oper;
    }
    bool hasOper(Oper op);
    void encodeAtoms(const SymAutomataNet *net);
    bool isECTLK(void) const;
};

using std::cout;

#define CTLK_COND_1ARG(a) ((a) == CTLK_NOT || (a) == CTLK_EG || (a) == CTLK_EF || (a) == CTLK_EX || (a) == CTLK_KD || (a) == CTLK_OD || (a) == CTLK_RT_EG || (a) == CTLK_RT_EF)
#define CTLK_COND_2ARG(a) ((a) == CTLK_AND || (a) == CTLK_OR || (a) == CTLK_IMPL || (a) == CTLK_EU || (a) == CTLK_RT_EU)

#endif
